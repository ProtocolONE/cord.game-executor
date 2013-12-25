/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Extension.h>

#include <GameExecutor/Executor/ExecutableFile_p.h>

#include <Core/System/HardwareId.h>
#include <RestApi/Commands/User/GetUserServiceAccount>

#include <QtCore/QCoreApplication>
#include <QMetaObject>

using RestApi::Commands::User::GetUserServiceAccount;
using RestApi::Commands::User::Response::UserServiceAccountResponse;

#define SERVICE_ID_SIZE_MAX 20

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

      ExecutableFilePrivate::ExecutableFilePrivate(QObject *parent) 
        : QObject(parent)
        , _serviceMapFileHandle(0)
        , _data(0)
      {
        connect(&this->_client, SIGNAL(started()), this, SLOT(launcherStarted()));
        connect(&this->_client, SIGNAL(finished(int)), this, SLOT(launcherFinished(int)));
      }

      ExecutableFilePrivate::~ExecutableFilePrivate()
      {
      }

      void ExecutableFilePrivate::setRestApiManager(GGS::RestApi::RestApiManager* manager) {
        this->_client.setRestApiManager(manager);
      }

      void ExecutableFilePrivate::execute(
        const GGS::Core::Service &service, 
        GameExecutorService *executorService,
        const GGS::RestApi::GameNetCredential& credential)
      {
        this->shareServiceId(service);        

        this->_service = service;
        QUrl url = this->_service.url();

        this->_path = url.path();
        this->_workingDir = url.queryItemValue("workingDir");
        this->_args = url.queryItemValue("args");

        QString injectDll = url.queryItemValue("injectDll");
        RestApi::GameNetCredential baseCredential = RestApi::RestApiManager::commonInstance()->credential();

        bool isSecondAccount = !credential.userId().isEmpty();

        if (isSecondAccount)
          this->_activityRequestArgs = 
            QString("%1|%2|%3|%4").arg(credential.userId(), credential.appKey(), service.gameId(), injectDll);
        else
          this->_activityRequestArgs = 
            QString("%1|%2|%3|%4").arg(baseCredential.userId(), baseCredential.appKey(), service.gameId(), injectDll);

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

        if (isSecondAccount) {
          this->_args.replace("%userId%", credential.userId(), Qt::CaseInsensitive);
          this->_args.replace("%appKey%", credential.appKey(), Qt::CaseInsensitive);
        } else {
          this->_args.replace("%userId%", baseCredential.userId(), Qt::CaseInsensitive);
          this->_args.replace("%appKey%", baseCredential.appKey(), Qt::CaseInsensitive);
        }

        if (-1 == this->_args.indexOf("%token%", 0, Qt::CaseInsensitive)
          && -1 == this->_args.indexOf("%login%", 0, Qt::CaseInsensitive)) {
            QMetaObject::invokeMethod(this, "launcherStart");
            return;
        }

        this->_executorService = executorService;
        
        GetSaltExtension *extension = reinterpret_cast<GetSaltExtension *>(
          this->_executorService->extension(ExtensionTypes::GetSalt));

        if (extension) 
          this->_authSalt = extension->get()();

        GetUserServiceAccount *cmd = new GetUserServiceAccount();
        cmd->setVersion("2");
        cmd->setServiceId(service.id());
        cmd->setHwid(GGS::Core::System::HardwareId::value());

        // UNDONE поидее когда запретим стартовать без драйвера эту проверку убрать надо
        if (this->_authSalt.size() > 0)
          cmd->appendParameter("salt", this->_authSalt);

        if (isSecondAccount) {
          cmd->appendParameter("secondUserId", credential.userId());
          cmd->appendParameter("secondAppKey", credential.appKey());
        }

        connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(getUserServiceAccountResult(GGS::RestApi::CommandBase::CommandResults)), Qt::DirectConnection);

        cmd->execute();
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

          QString token = response->getToken();

          GetTokenExtension *extension = reinterpret_cast<GetTokenExtension *>(
            this->_executorService->extension(ExtensionTypes::GetToken));

          if (extension) 
            token = extension->get()(this->_authSalt, token);

          this->_args.replace("%token%", token, Qt::CaseInsensitive);
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

      void ExecutableFilePrivate::launcherStarted()
      {
        emit this->started(this->_service);
      }

      void ExecutableFilePrivate::launcherFinished(int exitCode)
      {
        if (this->_data)
          UnmapViewOfFile(this->_data);
        
        if (this->_serviceMapFileHandle)
          CloseHandle(this->_serviceMapFileHandle);

        if (exitCode == 0) {
          emit this->finished(this->_service, Success);
          return;
        }

        CRITICAL_LOG << "with error code" << exitCode;

        QUrl url = this->_service.url();
        url.addQueryItem("exitCode", QString::number(exitCode));
        this->_service.setUrl(url);
        emit this->finished(this->_service, ExternalFatalError);
      }

      FinishState ExecutableFilePrivate::finishStateFromRestApiErrorCode(int errorCode)
      {
        FinishState state;

        switch(errorCode) {
        case RestApi::CommandBase::AuthorizationFailed:
        case RestApi::CommandBase::AccountNotExists:
        case RestApi::CommandBase::AuthorizationLimitExceed:
        case RestApi::CommandBase::UnknownAccountStatus:
          state = AuthorizationError;
          break;
        case RestApi::CommandBase::GuestExpired:
          state = GuestAccountExpired;
          break;
        case RestApi::CommandBase::ServiceAccountBlocked:
          state = ServiceAccountBlockedError;
          break;
        case RestApi::CommandBase::ServiceAuthorizationImpossible:
          state = ServiceAuthorizationImpossible;
          break;
        case RestApi::CommandBase::PakkanenPermissionDenied:
          state = PakkanenPermissionDenied;
          break;
        case RestApi::CommandBase::PakkanenPhoneVerification:
        case RestApi::CommandBase::PakkanenVkVerification:
        case RestApi::CommandBase::PakkanenVkPhoneVerification:
          state = PakkanenPhoneVerification;
          break;
        default:
          state = UnhandledRestApiError;
        }	

        return state;
      }

      void ExecutableFilePrivate::launcherStart()
      {
        QString message = 
          QString("start|%1|%2|%3|%4").arg(this->_path, this->_workingDir, this->_args, this->_activityRequestArgs);

        this->_client.sendMessage(message);
      }

      void ExecutableFilePrivate::shareServiceId(const GGS::Core::Service &service)
      {
        if (service.id().size() + 1 >= SERVICE_ID_SIZE_MAX) 
          return;

        this->_serviceMapFileHandle = CreateFileMappingA(
          INVALID_HANDLE_VALUE, 
          NULL, 
          PAGE_READWRITE, 
          0, 
          sizeof(unsigned int), 
          "Local\\GGSOverlayServiceId");

        if (!this->_serviceMapFileHandle)
          return;
         
        this->_data = MapViewOfFile(this->_serviceMapFileHandle, FILE_MAP_ALL_ACCESS, 0, 0, SERVICE_ID_SIZE_MAX);
        if (!this->_data)
          return;

        QByteArray serviceId = service.id().toAscii();
        memcpy_s(this->_data, SERVICE_ID_SIZE_MAX, serviceId.data(), serviceId.size());
        reinterpret_cast<char *>(this->_data)[serviceId.size()] = '\0';
      }

    }
  }
}