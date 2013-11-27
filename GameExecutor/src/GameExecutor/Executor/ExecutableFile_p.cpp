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
        , _ipcServerStarted(false)
        , _serviceMapFileHandle(0)
        , _data(0)
        , _syncJob(INVALID_HANDLE_VALUE)
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
        this->_process.close();
      }

      void ExecutableFilePrivate::execute(const GGS::Core::Service &service, GameExecutorService *executorService)
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

        this->shareServiceId(service);

        this->_service = service;
        QUrl url = this->_service.url();

        this->_path = url.path();
        this->_workingDir = url.queryItemValue("workingDir");
        this->_args = url.queryItemValue("args");

        QString injectDll = url.queryItemValue("injectDll");
        RestApi::GameNetCredential credential = RestApi::RestApiManager::commonInstance()->credential();

        this->_activityRequestArgs = 
          QString("%1|%2|%3|%4").arg(credential.userId(), credential.appKey(), service.gameId(), injectDll);

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

        this->_executorService = executorService;
        this->_authSalt = executorService->authSalt();

        GetUserServiceAccount *cmd = new GetUserServiceAccount();
        cmd->setVersion("2");
        cmd->setServiceId(service.id());
        cmd->setHwid(GGS::Core::System::HardwareId::value());
        
        if (this->_authSalt.size() > 0)
          cmd->appendParameter("salt", this->_authSalt);

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

          QString token = this->_executorService->authToken(this->_authSalt, response->getToken());
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

        _PROCESS_INFORMATION* pi = process->pid();
        DEBUG_LOG << "launcher process pid" << (pi ? pi->dwProcessId : 0);
        
        // 22.07.2013 Отключил по требованию.
        //this->_syncJob = CreateJobObject(NULL, NULL);
        //if (this->_syncJob) {
        //  JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
        //  jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        //  if (!SetInformationJobObject(this->_syncJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        //    DWORD res = GetLastError();
        //    DEBUG_LOG << "SetInformationJobObject error " << res;
        //  }

        //  if (!AssignProcessToJobObject(this->_syncJob, process->pid()->hProcess)) {
        //    DWORD res = GetLastError();
        //    DEBUG_LOG << "AssignProcessToJobObject error " << res;
        //  }

        //} else {
        //  DWORD res = GetLastError();
        //  DEBUG_LOG << "Create job error " << res;
        //}

        emit this->started(this->_service);
      }

      void ExecutableFilePrivate::launcherFinished(int exitCode, QProcess::ExitStatus exitStatus)
      {
        QProcess *process = qobject_cast<QProcess*>(QObject::sender());
        if (!process) {
          CRITICAL_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        if (this->_syncJob && this->_syncJob != INVALID_HANDLE_VALUE)
          CloseHandle(this->_syncJob);

        if (this->_data)
          UnmapViewOfFile(this->_data);
        
        if (this->_serviceMapFileHandle)
          CloseHandle(this->_serviceMapFileHandle);

        _PROCESS_INFORMATION* pi = process->pid();
        DEBUG_LOG << "launcher with pid" << (pi ? pi->dwProcessId : 0) << "finished";

        if (QProcess::NormalExit == exitStatus && exitCode == 0) {
          emit this->finished(this->_service, Success);
          return;
        }

        CRITICAL_LOG << "with error code" << exitCode;

        QUrl url = this->_service.url();
        url.addQueryItem("exitCode", QString::number(exitCode));
        this->_service.setUrl(url);
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
        this->_process.close();
#ifdef _DEBUG
        this->_process.start(QCoreApplication::applicationDirPath() + "/Launcherd.exe", QStringList() << this->_ipcName);
#else
        this->_process.start(QCoreApplication::applicationDirPath() + "/Launcher.exe", QStringList() << this->_ipcName);
#endif
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