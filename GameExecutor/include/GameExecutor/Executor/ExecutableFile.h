#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>

#include <Core/Service.h>

#include <QtCore/QHash>

using namespace P1;

namespace P1 {
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
      
      class ExecutableFilePrivate;

      class GAMEEXECUTOR_EXPORT ExecutableFile : public P1::GameExecutor::ExecutorBase
      {
        Q_OBJECT
      public:
        explicit ExecutableFile(QObject *parent = 0);
        virtual ~ExecutableFile();

        virtual void execute(
          const P1::Core::Service &service, 
          GameExecutorService *executorService,
          const P1::RestApi::GameNetCredential& credential) override;

      private:
        void internalFinished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);
      };
    }
  }
}

