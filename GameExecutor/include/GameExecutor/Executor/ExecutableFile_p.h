/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILEPRIVATE_H
#define _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILEPRIVATE_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <Core/Service>
#include <RestApi/CommandBase>

#include <QtCore/QProcess>

#include <Windows.h>

#include <random>

using namespace GGS;
using namespace RestApi;

namespace GGS {
  namespace GameExecutor {

    class GameExecutorService;

    namespace Executor {

      class ExecutableFilePrivate : public QObject
      {
        Q_OBJECT

      public:
        explicit ExecutableFilePrivate(QObject *parent);
        virtual ~ExecutableFilePrivate();

        void execute(const GGS::Core::Service &service, 
          GameExecutorService *executorService,
          const GGS::RestApi::GameNetCredential& credential);

      public slots:
        void shutdown();

      signals:
        void started(const GGS::Core::Service &service);
        void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);

      private slots:
        void launcherStart();
        void launcherStarted();
        void launcherFinished(int exitCode); 

        void closeSharedString();

        void getUserServiceAccountResult(GGS::RestApi::CommandBase::CommandResults result);

      private:
        void createAndExecuteLauncherProcess();
        FinishState finishStateFromRestApiErrorCode(int errorCode);
        void shareServiceId(const GGS::Core::Service &service);
        bool shareString(const std::wstring& name, const std::wstring& value);

        void shareStringForProcess(const std::wstring& name, const std::wstring& value, unsigned int pid, HANDLE handle);
        
        void prepairArgsEx(const QString& format, const QString& token);
        QString fakeToken(const QString& realToken);

        HANDLE _serviceMapFileHandle;
        LPVOID _data;

        HANDLE _stringShareHandle;
        LPVOID _stringShareData;

        GameExecutorService *_executorService;
        GGS::Core::Service _service;
        GGS::GameExecutor::Executor::ExecutableFileClient _client;

        QString _appPath;

        QString _path;
        QString _workingDir;
        QString _args;
        QString _activityRequestArgs;

        QString _authSalt;
        bool _executorHelperAvailable;
        std::wstring _argsEx;

        std::default_random_engine _random;
      };
    }
  }
}

#endif // _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILEPRIVATE_H
