/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#pragma once

#include <GameExecutor/gameexecutor_global.h>

#include <RestApi/RestApiManager>
#include <RestApi/CommandBase>

#include <QtCore/QObject>

#include <functional>
#include <Windows.h>

#include <map>
#include <string>

#include <memory>

namespace GameNetAuthHost {
  class AuthWriter;
}

namespace GGS {
  namespace GameExecutor {

    class GameExecutorService;

    namespace Executor {

      enum HookType
      {
        CommandLineCheck,
        SpeedHackCheck,
        Need64Load,
        WinHookDefendCheck
      };

      enum AuthData
      {
        UserIdData,
        AppKeyData,
        TokenData,
        UseAuthSdk
      };

      typedef std::map<HookType, bool> paramHolderT;
      typedef std::map<AuthData, QString> authParamT;

      class AppInitPatch;

      /*!
      \class ExecutableFileClient
      \brief Executable file client. 
      */
      class GAMEEXECUTOR_EXPORT ExecutableFileClient : public QObject
      {
        Q_OBJECT
      public:
        const static int Success = 0;  
        const static int Fail    = 1;

        explicit ExecutableFileClient(QObject *parent = 0);
        virtual ~ExecutableFileClient();

        void startProcess(
          const QString& pathToExe,
          const QString& args,
          const QString& workingDir,
          const QString& injectedDll);

        void stopProcess();

        void setShareArgs(std::function<void (unsigned int, HANDLE)> value);
        void setDeleteSharedArgs(std::function<void ()> value);
        void setInjectedParams(HookType key, bool val);
        void setServiceId(const QString& value);

        void setAuthParam(AuthData, const QString & value);
        void setService(GGS::GameExecutor::GameExecutorService *);

      signals:
        void finished(int exitCode);
        void started();
        void corruptedData();

      private:

        void handleActivated(HANDLE handle);
        void closeHandles();

        void initAuth(const quint32 pid);
        void closeAuth();
        bool isAuthSdkEnabled();

        std::unique_ptr<AppInitPatch> _appinitPatch;

        std::function<void (unsigned int, HANDLE)> _shareArgs;
        std::function<void ()> _deleteSharedArgs;

        HANDLE _processHandle;
        HANDLE _threadHandle;
        paramHolderT _paramHolder;
        authParamT _authParam;
        QString _serviceId;
        GameNetAuthHost::AuthWriter *_authWriter;
        GGS::GameExecutor::GameExecutorService *_executorService;
      };
    }
  }
}
