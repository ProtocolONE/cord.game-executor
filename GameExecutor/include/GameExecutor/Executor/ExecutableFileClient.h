#pragma once

#include <GameExecutor/gameexecutor_global.h>

#include <RestApi/RestApiManager.h>
#include <RestApi/CommandBase.h>

#include <QtCore/QObject>

#include <functional>
#include <Windows.h>

#include <map>
#include <string>

#include <memory>

namespace GameNetAuthHost {
  class AuthWriter;
}

namespace P1 {
  namespace GameExecutor {

    class GameExecutorService;

    namespace Executor {

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
          const QString& workingDir);

        void stopProcess();

      signals:
        void finished(int exitCode);
        void started();
      private:
        void handleActivated(HANDLE handle);
        void closeHandles();

        HANDLE _processHandle;
        HANDLE _threadHandle;
      };
    }
  }
}
