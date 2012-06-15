#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile.h>
#include <GameExecutor/Executor/ExecutableFile_p.h>

namespace GGS {
  namespace GameExecutor {
    namespace Executor {
      ExecutableFile::ExecutableFile(QObject *parent) 
        : ExecutorBase("exe", parent)
      {
      }

      ExecutableFile::~ExecutableFile()
      {
      }

      void ExecutableFile::setWorkingDirectory(const QString &dir)
      {
        this->_appPath = dir;
      }

      void ExecutableFile::execute(const Core::Service &service, GameExecutorService *executorService)
      {
        QString id = service.id();

        ExecutableFilePrivate* executor = new ExecutableFilePrivate(this);
        executor->setWorkingDirectory(this->_appPath);

        connect(executor, SIGNAL(started(const Core::Service &)), 
          this, SIGNAL(started(const Core::Service &)), Qt::DirectConnection);

        connect(executor, SIGNAL(finished(const Core::Service &, GGS::GameExecutor::FinishState)), 
          this, SLOT(internalFinished(const Core::Service &, GGS::GameExecutor::FinishState)), Qt::DirectConnection);
          
        executor->execute(service, executorService);
      }


      void ExecutableFile::internalFinished( const Core::Service &service, GGS::GameExecutor::FinishState state )
      {
        QObject::sender()->deleteLater();
        emit this->finished(service, state);
      }
    }
  }
}