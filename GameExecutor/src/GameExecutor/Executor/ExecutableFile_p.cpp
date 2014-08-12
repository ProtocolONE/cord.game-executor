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
#include <QtCore/QDateTime>
#include <QtCore/QUrlQuery>
#include <QMetaObject>

#include <sstream>

using RestApi::Commands::User::GetUserServiceAccount;
using RestApi::Commands::User::Response::UserServiceAccountResponse;

#define SERVICE_ID_SIZE_MAX 20
#define GARBAGE_COUNT 64
#define GARBAGE_INDEX_MASK 0x01CCFA1C

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

      ExecutableFilePrivate::ExecutableFilePrivate(QObject *parent) 
        : QObject(parent)
        , _serviceMapFileHandle(0)
        , _data(0)
        , _executorHelperAvailable(false)
        , _stringShareHandle(INVALID_HANDLE_VALUE)
      {
        connect(&this->_client, SIGNAL(started()), this, SLOT(launcherStarted()));
        connect(&this->_client, SIGNAL(finished(int)), this, SLOT(launcherFinished(int)));
        
        this->_random.seed(QDateTime::currentMSecsSinceEpoch());
      }

      ExecutableFilePrivate::~ExecutableFilePrivate()
      {
      }

      void ExecutableFilePrivate::execute(
        const GGS::Core::Service &service, 
        GameExecutorService *executorService,
        const GGS::RestApi::GameNetCredential& credential)
      {
        this->shareServiceId(service);        

        this->_service = service;
        QUrl url = this->_service.url();

        QUrlQuery urlQuery(url);

        this->_path = url.path();
        this->_workingDir = urlQuery.queryItemValue("workingDir", QUrl::FullyDecoded);
        this->_args = urlQuery.queryItemValue("args", QUrl::FullyDecoded);

        QString injectDll;
        if (urlQuery.queryItemValue("noinject") != "1")
          injectDll = urlQuery.queryItemValue("injectDll");

        QString executorHelper = urlQuery.queryItemValue("executorHelper", QUrl::FullyDecoded);
        this->_executorHelperAvailable = !executorHelper.isEmpty();

        RestApi::GameNetCredential baseCredential = RestApi::RestApiManager::commonInstance()->credential();

        bool isSecondAccount = !credential.userId().isEmpty();

        if (isSecondAccount)
          this->_activityRequestArgs = 
            QString("%1|%2|%3|%4|%5").arg(credential.userId(), credential.appKey(), service.gameId(), injectDll, executorHelper);
        else
          this->_activityRequestArgs = 
            QString("%1|%2|%3|%4|%5").arg(baseCredential.userId(), baseCredential.appKey(), service.gameId(), injectDll, executorHelper);

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

        cmd->appendParameter("thettaVersion", "1");

        if (isSecondAccount) {
          cmd->appendParameter("secondUserId", credential.userId());
          cmd->appendParameter("secondAppKey", credential.appKey());
        }

        connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(getUserServiceAccountResult(GGS::RestApi::CommandBase::CommandResults)), Qt::DirectConnection);

        cmd->execute();
      }
      
      void ExecutableFilePrivate::shutdown()
      {
        this->_client.stopProcess();
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

          QString tokenEx = "";
          if (extension) 
            tokenEx = extension->get()(this->_authSalt, token);
          else
            tokenEx = token;

          if (!this->_executorHelperAvailable)
            token = tokenEx;

          this->_args.replace("%login%", response->getLogin(), Qt::CaseInsensitive);

          if (this->_executorHelperAvailable) {
            this->prepairArgsEx(this->_args, tokenEx);
            tokenEx.fill('\n');

            this->_client.setShareArgs([this](unsigned int pid, HANDLE handle) {
              std::wstringstream name;
              name << L"Local\\HelperInfo_" << pid;
              this->shareStringForProcess(name.str(), this->_argsEx, pid, handle);
            });

            this->_client.setDeleteSharedArgs([this](){
              this->closeSharedString();
            });
          }

          this->_args.replace("%token%", token, Qt::CaseInsensitive);
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

        this->closeSharedString();

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
        QString startParams = 
          QString("start|%1|%2|%3|%4").arg(this->_path, this->_workingDir, this->_args, this->_activityRequestArgs);

        this->_client.startProcess(startParams);
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

        QByteArray serviceId = service.id().toLatin1();
        memcpy_s(this->_data, SERVICE_ID_SIZE_MAX, serviceId.data(), serviceId.size());
        reinterpret_cast<char *>(this->_data)[serviceId.size()] = '\0';
      }

      bool ExecutableFilePrivate::shareString(const std::wstring& name, const std::wstring& value)
      {
        // INFO простое лобовое шифрование для быстрой выливки. В будущем можно поменять на что-то посложнее.
        std::wstring valueEx;
        std::wstringstream ss;
        wchar_t t = 0x46;
        wchar_t k = 0xCE;
        for (int i = 0; i < value.size(); ++i) {
          wchar_t c0 = value.at(i);
          wchar_t c1 = c0 ^ k ^ t;
          t = c0;
          ss << c1;
        }

        valueEx = ss.str();

        unsigned int fullSize = sizeof(unsigned int) + valueEx.size() * sizeof(wchar_t);
        this->_stringShareHandle = CreateFileMappingW(
          INVALID_HANDLE_VALUE, 
          NULL, 
          PAGE_READWRITE, 
          0, 
          fullSize, 
          name.c_str());

        if (this->_stringShareHandle == INVALID_HANDLE_VALUE || this->_stringShareHandle == NULL)
          return false;

        this->_stringShareData = reinterpret_cast<char *>(
          MapViewOfFile(this->_stringShareHandle, FILE_MAP_ALL_ACCESS, 0, 0, fullSize));

        if (!this->_stringShareData) {
          CloseHandle(this->_stringShareHandle);
          this->_stringShareHandle = INVALID_HANDLE_VALUE;
          return false;
        }

        unsigned int *sizePtr = reinterpret_cast<unsigned int *>(this->_stringShareData);
        *sizePtr = valueEx.size();

        wchar_t *str = reinterpret_cast<wchar_t *>((char*)this->_stringShareData + sizeof(unsigned int));
        valueEx.copy(str, valueEx.size());

        return true;
      }

      void ExecutableFilePrivate::closeSharedString()
      {
        if (this->_stringShareData) {
          UnmapViewOfFile(this->_stringShareData);
          this->_stringShareData = NULL;
        }

        if (this->_stringShareHandle != INVALID_HANDLE_VALUE && this->_stringShareHandle != NULL) {
          CloseHandle(this->_stringShareHandle);
          this->_stringShareHandle = INVALID_HANDLE_VALUE;
        }
      }

      void ExecutableFilePrivate::shareStringForProcess(const std::wstring& name, const std::wstring& value, unsigned int pid, HANDLE handle)
      {
        ProcessHandleCheckExtension *extension = reinterpret_cast<ProcessHandleCheckExtension *>(
          this->_executorService->extension(ExtensionTypes::ProcessHandleCheck));

        if (extension->get()(pid, handle))
          this->shareString(name, value);
      }

      void ExecutableFilePrivate::prepairArgsEx(const QString& format, const QString& token)
      {
        int garbageCount = GARBAGE_COUNT;
        std::uniform_int_distribution<int> distribution(0, 0x00FFFFFF);

        int realIndex = 1 + (distribution(this->_random) % (garbageCount - 2));

        QString realFormat = format;
        std::wstring realArgs = realFormat.replace("%token%", token, Qt::CaseInsensitive).toStdWString();

        size_t totalSize = (2 * sizeof(unsigned int)) 
          + ((realArgs.size() + 1) * garbageCount * sizeof(wchar_t)) 
          + sizeof(wchar_t);

        std::vector<char> buffer(totalSize, 0);
        char* bufferData = &buffer[0];

        unsigned int* subStringSizePtr = reinterpret_cast<unsigned int*>(bufferData);
        int* garbageIndexPtr = reinterpret_cast<int*>(bufferData + sizeof(unsigned int));
        wchar_t * subStringPtr = reinterpret_cast<wchar_t *>(bufferData + sizeof(unsigned int) * 2);

        *subStringSizePtr = realArgs.size() + 1;
        *garbageIndexPtr = (realIndex ^ GARBAGE_INDEX_MASK);

        for (int i = 0; i < garbageCount; ++i) {
          if (i == realIndex) {
            realArgs.copy(subStringPtr, realArgs.size());
          } else {
            QString tmp = format;
            std::wstring arg = tmp.replace("%token%", this->fakeToken(token), Qt::CaseInsensitive).toStdWString();
            arg.copy(subStringPtr, arg.size());
          }

          subStringPtr += realArgs.size();
          *subStringPtr = L'\0';
          ++subStringPtr;
        }

        realArgs.assign(realArgs.size(), L'\t');
        this->_argsEx.assign(reinterpret_cast<const wchar_t*>(bufferData), totalSize / sizeof(wchar_t));
      }

      QString ExecutableFilePrivate::fakeToken(const QString& realToken)
      {
        const wchar_t str[16] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', 
                                L'8', L'9', L'a', L'b', L'c', L'd', L'e', L'f' };

        std::uniform_int_distribution<int> distribution(0, 15);

        std::wstring result;

        for (int i = 0; i < realToken.size(); ++i) {
          int rnd = distribution(this->_random);
          result.append(1, str[rnd]);
        }

        return QString::fromStdWString(result);
      }

    }
  }
}