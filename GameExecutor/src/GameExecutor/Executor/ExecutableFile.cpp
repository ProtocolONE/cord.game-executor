#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile.h>
#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <RestApi/Commands/User/GetUserServiceAccount>

#include <QtCore/QRegExp>

using RestApi::Commands::User::GetUserServiceAccount;
using RestApi::Commands::User::Response::UserServiceAccountResponse;

namespace GGS {
  namespace GameExecutor {
    namespace Executor {
      ExecutableFile::ExecutableFile(QObject *parent) 
        : ExecutorBase(parent), _ipcServerStarted(false)
      {
        this->_scheme = QString("exe");

        connect(this, SIGNAL(internalStart()), this, SLOT(launcherStart()));

        connect(&this->_process, SIGNAL(started()), this, SLOT(launcherStarted()));
        connect(&this->_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
          this, SLOT(launcherFinished(int, QProcess::ExitStatus)));
        connect(&this->_process, SIGNAL(error(QProcess::ProcessError)), 
          this, SLOT(launcherError(QProcess::ProcessError)));

        this->_ipcServer.setName(QString(GAMEEXECUTOR_IPC_CHANNEL));
        connect(&this->_ipcServer, SIGNAL(messageReceived(int, QString)), 
          this, SLOT(launcerMessageReceived(int, QString)));

        connect(&this->_ipcServer, SIGNAL(clientConnected(int)), 
          this, SLOT(launcherConnected(int)));

        this->_ipcServerStarted = this->_ipcServer.start();
      }

      ExecutableFile::~ExecutableFile()
      {
      }

      void ExecutableFile::execute(const Core::Service &service, GameExecutorService *executorService)
      {
        if (false == this->_ipcServerStarted) {
          emit this->finished(this->_service, InternalFatalError);
          return;
        }

        this->_service = service;

        QUrl url = service.url();

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

        if (-1 == this->_args.indexOf("%token%", 0, Qt::CaseInsensitive)) {
          emit this->internalStart();
          return;
        }

        GetUserServiceAccount *cmd = new GetUserServiceAccount();
        cmd->setServiceId(service.id());

        connect(cmd, SIGNAL(result(GGS::RestApi::CommandBaseInterface::CommandResults)), 
          this, SLOT(getUserServiceAccountResult(GGS::RestApi::CommandBaseInterface::CommandResults)));

        mgr->execute(cmd);
      }

      void ExecutableFile::getUserServiceAccountResult(CommandBaseInterface::CommandResults result)
      {
        GetUserServiceAccount *cmd = qobject_cast<GetUserServiceAccount*>(QObject::sender());
        if (!cmd) {
          qCritical() << __LINE__ << __FUNCTION__ << QObject::sender()->metaObject()->className();
          return;
        }

        cmd->deleteLater();  

        if (result == CommandBaseInterface::NoError) {
          UserServiceAccountResponse *response = cmd->response();

          this->_args.replace("%token%", response->getToken(), Qt::CaseInsensitive);
          emit this->internalStart();
          return;
        }

        int errorCode = cmd->getGenericErrorMessageCode();
       
        FinishState state = (CommandBaseInterface::GenericError == result) 
          ? this->finishStateFromRestApiErrorCode(errorCode) : ExternalFatalError;
        
        qCritical() << "getUserServiceAccountResult " << errorCode;
        
        this->finished(this->_service, state);
      }

      void ExecutableFile::launcherError(QProcess::ProcessError error)
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          qCritical() << __LINE__ << __FUNCTION__;
          return;
        }

        qCritical() << __LINE__ << __FUNCTION__ << error;
        this->finished(this->_service, ExternalFatalError);
      }

      void ExecutableFile::launcherStarted()
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          qCritical() << __LINE__ << __FUNCTION__;
          return;
        }

        qDebug() << __FUNCTION__ << process->pid();

        emit this->started(this->_service);
      }

      void ExecutableFile::launcherFinished(int exitCode, QProcess::ExitStatus exitStatus)
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          qCritical() << __LINE__ << __FUNCTION__;
          return;
        }

        qDebug() << __LINE__ << __FUNCTION__ << process->pid();

        if (QProcess::NormalExit == exitStatus && exitCode == 0) {
          emit this->finished(this->_service, Success);
          return;
        }

        qCritical() << __LINE__ << __FUNCTION__  << exitCode;

        emit this->finished(this->_service, ExternalFatalError);
      }

      void ExecutableFile::launcerMessageReceived(int id, QString message)
      {
        Q_ASSERT(this->_processIpcId == id);
        qDebug() << __LINE__ << __FUNCTION__ << message;
      }

      void ExecutableFile::launcherConnected( int id )
      {
        qDebug() << __LINE__ << __FUNCTION__;
        this->_processIpcId = id;

        QString message = 
          QString("start|%1|%2|%3|%4").arg(this->_path, this->_workingDir, this->_args, this->_activityRequestArgs);

        this->_ipcServer.sendMessage(this->_processIpcId, message);
      }

      void ExecutableFile::setWorkingDirectory( const QString &dir )
      {
        this->_process.setWorkingDirectory(dir);
      }

      FinishState ExecutableFile::finishStateFromRestApiErrorCode(int errorCode)
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

      void ExecutableFile::launcherStart()
      {
#ifdef _DEBUG
        this->_process.start(QCoreApplication::applicationDirPath() + "/Launcherd.exe");
#else
        this->_process.start(QCoreApplication::applicationDirPath() + "/Launcher.exe");
#endif
      }
    }
  }
}