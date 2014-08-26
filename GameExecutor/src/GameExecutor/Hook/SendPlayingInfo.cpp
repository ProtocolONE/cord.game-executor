#include <GameExecutor/Hook/SendPlayingInfo.h>
#include <RestApi/Commands/User/SetUserActivity.h>

#include <QtCore/QTimer>

using GGS::RestApi::Commands::User::SetUserActivity;
using GGS::RestApi::CommandBase;

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      SendPlayingInfo::SendPlayingInfo(QObject *parent /*= 0*/)
        : HookInterface(parent)
        , _playingCmd(nullptr)
        , _timer(new QTimer(this))
      {
        this->_timer->setSingleShot(true);
        QObject::connect(this->_timer, &QTimer::timeout,
          this, &SendPlayingInfo::sendPlaying);
      }

      SendPlayingInfo::~SendPlayingInfo()
      {
      }

      void SendPlayingInfo::PreExecute(Core::Service &service)
      {
        bool gameIdCastResult = false;
        int gameId = service.gameId().toInt(&gameIdCastResult);
        if (!gameIdCastResult) {
          emit this->preExecuteCompleted(service, GGS::GameExecutor::InternalFatalError);
          return;
        }

        this->_playingCmd = new SetUserActivity();
        this->_playingCmd->setGameId(gameId);
        this->_playingCmd->setLogout(0);
        this->_playingCmd->setAuthRequire(false);
        this->_playingCmd->appendParameter("userId", this->_credential.userId());
        this->_playingCmd->appendParameter("appKey", this->_credential.appKey());

        QObject::connect(this->_playingCmd, &SetUserActivity::result,
          this, &SendPlayingInfo::playingResult);

        this->sendPlaying();

        emit this->preExecuteCompleted(service, GGS::GameExecutor::Success);
      }

      void SendPlayingInfo::PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state)
      {
        this->_timer->stop();
        this->_playingCmd->deleteLater();
        this->_playingCmd = 0;

        int gameId = service.gameId().toInt();

        SetUserActivity *cmd = new SetUserActivity();
        cmd->setGameId(gameId);
        cmd->setLogout(1);
        cmd->setAuthRequire(false);
        cmd->appendParameter("userId", this->_credential.userId());
        cmd->appendParameter("appKey", this->_credential.appKey());
        QObject::connect(cmd, &SetUserActivity::result, [cmd]() { 
          cmd->deleteLater(); 
        });

        cmd->execute();
        emit this->postExecuteCompleted(service);
      }

      void SendPlayingInfo::sendPlaying()
      {
        this->_playingCmd->execute();
      }

      void SendPlayingInfo::playingResult(GGS::RestApi::CommandBase::CommandResults result)
      {
        SetUserActivity *cmd = qobject_cast<SetUserActivity *>(QObject::sender());
        int timeout = 300000;

        if (result == CommandBase::NoError) {
          int newTimeout = cmd->getTimeout();
          if (newTimeout > 0) {
            timeout = newTimeout * 1000;
          }
        }

        this->_timer->start(timeout);
      }

      QString SendPlayingInfo::id()
      {
        return "4188E79A-0C5A-482F-9373-963963450543";
      }

    }
  }
}