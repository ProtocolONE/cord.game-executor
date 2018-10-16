#include <GameExecutor/GameExecutorService.h>

#include <GameExecutor/Executor/ExecutableFile_p.h>

#include <Core/System/HardwareId.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QUrlQuery>
#include <QtCore/QDebug>


namespace P1 {
  namespace GameExecutor {
    namespace Executor {

      ExecutableFilePrivate::ExecutableFilePrivate(QObject *parent) 
        : QObject(parent)
      {
        QObject::connect(&this->_client, &ExecutableFileClient::started, 
          this, &ExecutableFilePrivate::launcherStarted);
        
        QObject::connect(&this->_client, &ExecutableFileClient::finished, 
          this, &ExecutableFilePrivate::launcherFinished);
      }

      void ExecutableFilePrivate::execute(
        const P1::Core::Service &service, 
        GameExecutorService *executorService,
        const P1::RestApi::ProtocolOneCredential& credential)
      {
        QString userIdTest;
        QString appKeyTest;
        QString tokenTest;

        this->_service = service;
        QUrl url = this->_service.url();

        QUrlQuery urlQuery(url);

        this->_path = url.path();
        this->_workingDir = urlQuery.queryItemValue("workingDir", QUrl::FullyDecoded);
        this->_args = urlQuery.queryItemValue("args", QUrl::FullyDecoded);

        QRegExp rx("%(.+)%");
        rx.setMinimal(true);

        int pos = 0;
        while ((pos = rx.indexIn(this->_args, pos)) != -1) {
          pos += rx.matchedLength();
          QString entry = rx.cap(1);
          if (urlQuery.hasQueryItem(entry)) {
            this->_args.replace("%" + entry + "%", urlQuery.queryItemValue(entry, QUrl::FullyDecoded), Qt::CaseInsensitive);
          }
        }
        
        this->_args.replace("%userId%", credential.userId(), Qt::CaseInsensitive);
        this->launcherStart();


        // INFO Auth unsupported for now.

        //bool needAuth = 
        //  -1 != this->_args.indexOf("%token%", 0, Qt::CaseInsensitive)
        //  || -1 != this->_args.indexOf("%login%", 0, Qt::CaseInsensitive);

        //this->_executorService = executorService;

        //if (!needAuth) {
        //  this->launcherStart();
        //  return;
        //}

        //GetUserServiceAccount *cmd = new GetUserServiceAccount();
        //cmd->setVersion("1");
        //cmd->setServiceId(service.id());
        //cmd->setHwid(P1::Core::System::HardwareId::value());

        //cmd->setAuthRequire(false);
        //cmd->appendParameter("userId", credential.userId());
        //cmd->appendParameter("appKey", credential.appKey());

        //QObject::connect(cmd, &GetUserServiceAccount::result,
        //  this, &ExecutableFilePrivate::getUserServiceAccountResult, Qt::DirectConnection);

        //cmd->execute();
      }
      
      void ExecutableFilePrivate::shutdown(const QString& serviceId)
      {
        if (!serviceId.isEmpty() && this->_service.id() != serviceId)
          return;

        this->_client.stopProcess();
      }

      //void ExecutableFilePrivate::getUserServiceAccountResult(P1::RestApi::CommandBase::CommandResults result)
      //{
      //  GetUserServiceAccount *cmd = qobject_cast<GetUserServiceAccount*>(QObject::sender());
      //  if (!cmd)
      //    return;

      //  cmd->deleteLater();  

      //  if (result == P1::RestApi::CommandBase::NoError) {
      //    UserServiceAccountResponse *response = cmd->response();

      //    this->_args.replace("%login%", response->getLogin(), Qt::CaseInsensitive);
      //    this->_args.replace("%token%", response->getToken(), Qt::CaseInsensitive);
      //    this->launcherStart();
      //    return;
      //  }

      //  int errorCode = cmd->getGenericErrorMessageCode();

      //  FinishState state = (P1::RestApi::CommandBase::GenericError == result)
      //    ? ExecutorBase::finishStateFromRestApiErrorCode(errorCode) : ExternalFatalError;

      //  CRITICAL_LOG << "with error code" << errorCode;
      //  this->finished(this->_service, state);
      //}

      void ExecutableFilePrivate::launcherStarted()
      {
        emit this->started(this->_service);
      }

      void ExecutableFilePrivate::launcherFinished(int exitCode)
      {
        if (exitCode == 0) {
          emit this->finished(this->_service, Success);
          return;
        }

        CRITICAL_LOG << "with error code" << exitCode;

        QUrl url = this->_service.url();
        QUrlQuery urlQuery(url);
        urlQuery.addQueryItem("exitCode", QString::number(exitCode));
        url.setQuery(urlQuery);

        this->_service.setUrl(url);
        emit this->finished(this->_service, ExternalFatalError);
      }

      void ExecutableFilePrivate::launcherStart()
      {
        this->_client.startProcess(
          this->_path, 
          this->_args,
          this->_workingDir);
      }
    }
  }
}