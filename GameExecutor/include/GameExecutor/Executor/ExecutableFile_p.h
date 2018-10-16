#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <Core/Service.h>

#include <QtCore/QProcess>

#include <Windows.h>

#include <random>

namespace P1 {
  namespace GameExecutor {

    class GameExecutorService;

    namespace Executor {

      class ExecutableFilePrivate : public QObject
      {
        Q_OBJECT

      public:
        explicit ExecutableFilePrivate(QObject *parent);
        virtual ~ExecutableFilePrivate() = default;

        void execute(
          const P1::Core::Service &service, 
          GameExecutorService *executorService,
          const P1::RestApi::ProtocolOneCredential& credential);

      public slots:
        void shutdown(const QString& serviceId);

      signals:
        void started(const P1::Core::Service &service);
        void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);

      private:
        void launcherStart();
        void launcherStarted();
        void launcherFinished(int exitCode);

        //void getUserServiceAccountResult(P1::RestApi::CommandBase::CommandResults result);

        GameExecutorService *_executorService;
        P1::Core::Service _service;
        P1::GameExecutor::Executor::ExecutableFileClient _client;

        QString _appPath;

        QString _path;
        QString _workingDir;
        QString _args;
      };
    }
  }
}
