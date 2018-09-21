

#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile.h>
#include <GameExecutor/Executor/ExecutableFile_p.h>

namespace P1 {
  namespace GameExecutor {
    namespace Executor {
      ExecutableFile::ExecutableFile(QObject *parent) 
        : ExecutorBase("exe", parent)
      {
      }

      ExecutableFile::~ExecutableFile()
      {
      }

      void ExecutableFile::execute(
        const P1::Core::Service &service, 
        GameExecutorService *executorService,
        const P1::RestApi::ProtocolOneCredential& credential)
      {
        ExecutableFilePrivate* executor = new ExecutableFilePrivate(this);

        QObject::connect(executor, &ExecutableFilePrivate::started,
          this, &ExecutableFile::started, Qt::DirectConnection);

        QObject::connect(executor, &ExecutableFilePrivate::finished,
          this, &ExecutableFile::internalFinished, Qt::DirectConnection);

        QObject::connect(executorService, &GameExecutorService::stopExecution,
          executor, &ExecutableFilePrivate::shutdown);

        executor->execute(service, executorService, credential);
      }

      void ExecutableFile::internalFinished(const P1::Core::Service &service, P1::GameExecutor::FinishState state)
      {
        QObject::sender()->deleteLater();
        emit this->finished(service, state);
      }
    }
  }
}