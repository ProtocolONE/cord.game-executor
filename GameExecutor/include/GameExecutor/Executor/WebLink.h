#pragma once
#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>

#include <QObject>

namespace P1 {
  namespace GameExecutor {
    namespace Executor {

      /*!
      \class ExecutableFile
      \brief Позволяет запустить игру, работающую по схеме `http`. 

      \code
      QUrl url("http://www.playga.ru");
      
      Core::Service service;
      service.setUrl(url);

      WebLink executor;
      executor.execute();
      \endcode

      \sa Core::Service, QUrl
      */
      class GAMEEXECUTOR_EXPORT WebLink : public P1::GameExecutor::ExecutorBase
      {
        Q_OBJECT
      public:
        explicit WebLink(QObject *parent = 0);
        virtual ~WebLink();

        virtual void execute(
          const P1::Core::Service &service, 
          GameExecutorService *executorService,
          const P1::RestApi::GameNetCredential& credential) override;

      };
    }
  }
}
