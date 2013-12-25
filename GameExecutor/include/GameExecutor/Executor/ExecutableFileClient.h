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
#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>

using GGS::RestApi::CommandBase;

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
        
        void sendMessage(QString message);

      signals:
        void exit(int code);
        void finished(int exitCode);
        void started();

      private slots:
        void processFinished();

        void setUserActivity();
        void setUserActivityResult(GGS::RestApi::CommandBase::CommandResults result);
        void setUserActivityLogoutResult(GGS::RestApi::CommandBase::CommandResults result);
        
      private:
        void setUserActivityLogout(int code);
        unsigned int startProcess(const QString& pathToExe, const QString& workDirectory, const QString& args, const QString& dllPath = QString());

        RestApi::RestApiManager *_restApiManager;
        int _gameId;
        int _code;
        QFuture<unsigned int> _executeFeature;
        QFutureWatcher<unsigned int> _executeFeatureWatcher;
        
      };
    }
  }
}
