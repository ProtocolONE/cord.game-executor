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
#include <GameExecutor/IPC/Server.h>

#include <Core/Service>
#include <RestApi/CommandBase>

#include <QtCore/QProcess>

using namespace GGS;
using namespace RestApi;

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

      class ExecutableFilePrivate : public QObject
      {
        Q_OBJECT

      public:
        explicit ExecutableFilePrivate(QObject *parent);
        virtual ~ExecutableFilePrivate();

        void execute(const GGS::Core::Service &service, GameExecutorService *executorService);

        void setWorkingDirectory(const QString &dir);
      
      signals:
        void started(const GGS::Core::Service &service);
        void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);

      private slots:
        void launcherStart();
        void launcherError(QProcess::ProcessError error);
        void launcherStarted();
        void launcherFinished(int exitCode, QProcess::ExitStatus exitStatus);

        void launcherMessageReceived(int id, QString message);
        void launcherConnected(int id);

        void getUserServiceAccountResult(GGS::RestApi::CommandBase::CommandResults result);

      private:
        void createAndExecuteLauncherProcess();
        FinishState finishStateFromRestApiErrorCode(int errorCode);

        GGS::Core::Service _service;

        QProcess _process;
        int _processIpcId;

        QString _appPath;

        QString _path;
        QString _workingDir;
        QString _args;
        QString _activityRequestArgs;

        QString _ipcName;

        IPC::Server _ipcServer;
        bool _ipcServerStarted;
      };
    }
  }
}

#endif // _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILEPRIVATE_H
