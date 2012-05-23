/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILECLIENT_H
#define _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILECLIENT_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/IPC/Client.h>

#include <RestApi/RestApiManager>
#include <RestApi/CommandBaseInterface>

#include <QObject>
#include <QtCore/QProcess>

using GGS::RestApi::CommandBaseInterface;
using namespace GGS::GameExecutor;

namespace GGS{
  namespace GameExecutor{
    namespace Executor {

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
        ~ExecutableFileClient();

        void setRestApiManager(RestApi::RestApiManager *restApiManager);
        RestApi::RestApiManager *respApiManager();

        void exec();

      signals:
        void exit(int code);

      private slots:
        void connectedToServer();
        void disconnectedOrError();
        void messageFromServer(QString message);

        void processError(QProcess::ProcessError error);
        void processStarted();
        void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

        /*!
        \fn void ExecutableFileClient::checkProcessIsAlive();

        \brief Проверяет, что запущенный процесс по прежнему находится в списке запущенных.

        Система безопасности некоторых игр, например MW2 портит handle запущенного приложение и слот processFinished
        уже не будут вызван. Для этого случая и используется эта функция. 
        */
        void checkProcessIsAlive();

        void setUserActivity();
        void setUserActivityResult(GGS::RestApi::CommandBaseInterface::CommandResults result);
        void setUserActivityLogoutResult(GGS::RestApi::CommandBaseInterface::CommandResults result);
      private:
        void setUserActivityLogout(int code);

        RestApi::RestApiManager *_restApiManager;

        IPC::Client _client;
        QProcess _gameProcess;
        Q_PID _pid;
        int _gameId;
        int _code;
        bool _finishPassed;
      };
    }
  }
}
#endif // _GGS_GAMEEXECUTOR_EXECUTOR_EXECUTABLEFILECLIENT_H
