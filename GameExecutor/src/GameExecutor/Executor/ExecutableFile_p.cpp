#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile_p.h>

#include <RestApi/Commands/User/GetUserServiceAccount>

#include <QtCore/QCoreApplication>
#include <QMetaObject>

using RestApi::Commands::User::GetUserServiceAccount;
using RestApi::Commands::User::Response::UserServiceAccountResponse;

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

      ExecutableFilePrivate::ExecutableFilePrivate(QObject *parent) 
        : QObject(parent), _ipcServerStarted(false)
      {
        connect(&this->_process, SIGNAL(started()), this, SLOT(launcherStarted()));
        connect(&this->_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
          this, SLOT(launcherFinished(int, QProcess::ExitStatus)));
        connect(&this->_process, SIGNAL(error(QProcess::ProcessError)), 
          this, SLOT(launcherError(QProcess::ProcessError)));

        connect(&this->_ipcServer, SIGNAL(messageReceived(int, QString)), 
          this, SLOT(launcherMessageReceived(int, QString)));

        connect(&this->_ipcServer, SIGNAL(clientConnected(int)), 
          this, SLOT(launcherConnected(int)));
      }

      ExecutableFilePrivate::~ExecutableFilePrivate()
      {
      }

      void ExecutableFilePrivate::execute(const Core::Service &service, GameExecutorService *executorService)
      {
        if (!this->_ipcServerStarted) {
          this->_ipcName = QString("QGNA_GAMEEXECUTOR_IPC_CHANNEL:%1").arg(service.id());
          this->_ipcServer.setName(this->_ipcName);
          this->_ipcServerStarted = this->_ipcServer.start();
          
          if (!this->_ipcServerStarted) {
            emit this->finished(this->_service, InternalFatalError);
            return;
          }
        }

        this->_service = service;
        QUrl url = this->_service.url();

        this->_path = url.path();
        this->_workingDir = url.queryItemValue("workingDir");
        this->_args = url.queryItemValue("args");

        RestApi::RestApiManager *mgr = executorService->respApiManager();
        RestApi::GameNetCredential credential = mgr->credential();

        this->_activityRequestArgs = 
          QString("%1|%2|%3").arg(credential.userId(), credential.appKey(), service.gameId());

        QRegExp rx("%(.+)%");
        rx.setMinimal(true);

        int pos = 0;
        while ((pos = rx.indexIn(this->_args, pos)) != -1) {
          pos += rx.matchedLength();
          QString entry = rx.cap(1);
          if (url.hasQueryItem(entry)) {
            this->_args.replace("%" + entry + "%", url.queryItemValue(entry), Qt::CaseInsensitive);
          }
        }

        this->_args.replace("%userId%", credential.userId(), Qt::CaseInsensitive);
        this->_args.replace("%appKey%", credential.appKey(), Qt::CaseInsensitive);
        
        if (-1 == this->_args.indexOf("%token%", 0, Qt::CaseInsensitive)
          && -1 == this->_args.indexOf("%login%", 0, Qt::CaseInsensitive)) {
            QMetaObject::invokeMethod(this, "launcherStart");
            return;
        }

        GetUserServiceAccount *cmd = new GetUserServiceAccount();
        cmd->setServiceId(service.id());

        connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(getUserServiceAccountResult(GGS::RestApi::CommandBase::CommandResults)), Qt::DirectConnection);

        mgr->execute(cmd);
      }

      void ExecutableFilePrivate::getUserServiceAccountResult(CommandBase::CommandResults result)
      {
        GetUserServiceAccount *cmd = qobject_cast<GetUserServiceAccount*>(QObject::sender());
        if (!cmd) {
          CRITICAL_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        cmd->deleteLater();  

        if (result == CommandBase::NoError) {
          UserServiceAccountResponse *response = cmd->response();

          this->_args.replace("%token%", response->getToken(), Qt::CaseInsensitive);
          this->_args.replace("%login%", response->getLogin(), Qt::CaseInsensitive);
          QMetaObject::invokeMethod(this, "launcherStart");
          return;
        }

        int errorCode = cmd->getGenericErrorMessageCode();

        FinishState state = (CommandBase::GenericError == result) 
          ? this->finishStateFromRestApiErrorCode(errorCode) : ExternalFatalError;

        CRITICAL_LOG << "with error code" << errorCode;

        this->finished(this->_service, state);
      }

      void ExecutableFilePrivate::launcherError(QProcess::ProcessError error)
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          CRITICAL_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        CRITICAL_LOG << "with error code" << error;
        this->finished(this->_service, ExternalFatalError);
      }

      void ExecutableFilePrivate::launcherStarted()
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          CRITICAL_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        DEBUG_LOG << "launcher process pid" << process->pid();

        emit this->started(this->_service);
      }

      void ExecutableFilePrivate::launcherFinished(int exitCode, QProcess::ExitStatus exitStatus)
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          CRITICAL_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        DEBUG_LOG << "launcher with pid" << process->pid() << "finished";

        if (QProcess::NormalExit == exitStatus && exitCode == 0) {
          emit this->finished(this->_service, Success);
          return;
        }

        CRITICAL_LOG << "with error code" << exitCode;
        emit this->finished(this->_service, ExternalFatalError);
      }

      void ExecutableFilePrivate::launcherMessageReceived(int id, QString message)
      {
        Q_ASSERT(this->_processIpcId == id);
        DEBUG_LOG << "message from launcher" << message;
      }

      void ExecutableFilePrivate::launcherConnected(int id)
      {
        DEBUG_LOG << "with id" << id;
        this->_processIpcId = id;

        QString message = 
          QString("start|%1|%2|%3|%4").arg(this->_path, this->_workingDir, this->_args, this->_activityRequestArgs);

        this->_ipcServer.sendMessage(this->_processIpcId, message);
      }

      void ExecutableFilePrivate::setWorkingDirectory( const QString &dir )
      {
        this->_process.setWorkingDirectory(dir);
      }

      FinishState ExecutableFilePrivate::finishStateFromRestApiErrorCode(int errorCode)
      {
        FinishState state;

        switch(errorCode) {
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
          state = AuthorizationError;
          break;
        case 125:
          state = ServiceAccountBlockedError;
          break;
        default:
          state = UnhandledRestApiError;
        }	

        return state;
      }

      void ExecutableFilePrivate::launcherStart()
      {
        this->_process.close();
#ifdef _DEBUG
        this->_process.start(QCoreApplication::applicationDirPath() + "/Launcherd.exe", QStringList() << this->_ipcName);
#else
        this->_process.start(QCoreApplication::applicationDirPath() + "/Launcher.exe", QStringList() << this->_ipcName);
#endif
      }
    }
  }
}