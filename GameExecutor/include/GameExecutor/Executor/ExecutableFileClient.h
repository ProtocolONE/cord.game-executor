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

        void startProcess(
          const QString& pathToExe,
          const QString& args,
          const QString& workingDir,
          const QString& injectedDll,
          const QString& injectedDll2);

        void stopProcess();

        void setShareArgs(std::function<void (unsigned int, HANDLE)> value);
        void setDeleteSharedArgs(std::function<void ()> value);

      signals:
        void finished(int exitCode);
        void started();

      private:
        void injectDll(HANDLE handle, const QString& path, const QString& waitEvent = QString());
        void handleActivated(HANDLE handle);
        void closeHandles();

        AppInitPatch *_appinitPatch;

        std::function<void (unsigned int, HANDLE)> _shareArgs;
        std::function<void ()> _deleteSharedArgs;

        HANDLE _processHandle;
        HANDLE _threadHandle;
      };
    }
  }
}
