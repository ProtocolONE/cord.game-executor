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

#include <functional>
#include <Windows.h>

using GGS::RestApi::CommandBase;

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

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
        ~ExecutableFileClient();

        void startProcess(QString startParams);
        void stopProcess();

        void setShareArgs(std::function<void (unsigned int, HANDLE)> value);
        void setDeleteSharedArgs(std::function<void ()> value);

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
        unsigned int startProcessInternal(
          QString pathToExe, 
          QString workDirectory, 
          QString args, 
          QString dllPath = QString(),
          QString dllPath2 = QString());

        void injectDll(HANDLE handle, const QString& path, const QString& waitEvent = QString());

        AppInitPatch *_appinitPatch;

        QString _userId;
        QString _appKey;
        int _gameId;
        int _code;
        QFuture<unsigned int> _executeFeature;
        QFutureWatcher<unsigned int> _executeFeatureWatcher;

        std::function<void (unsigned int, HANDLE)> _shareArgs;
        std::function<void ()> _deleteSharedArgs;

        HANDLE _processHandle;
      };
    }
  }
}
