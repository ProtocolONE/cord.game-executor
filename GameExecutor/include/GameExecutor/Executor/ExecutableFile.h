/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILE_H
#define _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILE_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/IPC/Server.h>

#include <Core/Service>
#include <RestApi/CommandBaseInterface>

#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>

using namespace GGS;
using namespace RestApi;

namespace GGS {
  namespace GameExecutor {
    namespace Executor {
      /*!
        \class ExecutableFile
        \brief Позволяет запустить игру, работающую по схеме `exe`

        Позволяет запустить игру, работающую по схеме `exe`. 

        \code
        QUrl url;
        url.setScheme("exe");
        url.setPath("c:/folder/name.exe");
        url.addQueryItem("workingDir", "c:/folder/");
        url.addQueryItem("args", " -userId=%userId% %appKey%%token% %someKey%");
        url.addQueryItem("someKey", "...");

        Core::Service service;
        service.setUrl(url);

        ExecutableFile executor;
        executor.execute();
        \endcode

        \sa Core::Service, QUrl
      */
      class GAMEEXECUTOR_EXPORT ExecutableFile : public GGS::GameExecutor::ExecutorBase
      {
        Q_OBJECT
      public:
        explicit ExecutableFile(QObject *parent = 0);
        ~ExecutableFile();

        void execute(const Core::Service &service, GameExecutorService *executorService);

        void setWorkingDirectory(const QString &dir);
        
      signals:
        void internalStart();
      private slots:
        void launcherStart();
        void launcherError(QProcess::ProcessError error);
        void launcherStarted();
        void launcherFinished(int exitCode, QProcess::ExitStatus exitStatus);
        
        void launcerMessageReceived(int id, QString message);
        void launcherConnected(int id);

        void getUserServiceAccountResult(GGS::RestApi::CommandBaseInterface::CommandResults result);

      private:
        void createAndExecuteLauncherProcess();
        FinishState finishStateFromRestApiErrorCode(int errorCode);

        Core::Service _service;

        QProcess _process;
        int _processIpcId;

        QString _appPath;

        QString _path;
        QString _workingDir;
        QString _args;
        QString _activityRequestArgs;

        IPC::Server _ipcServer;
        bool _ipcServerStarted;
      };
    }
  }
}

#endif // _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILE_H
