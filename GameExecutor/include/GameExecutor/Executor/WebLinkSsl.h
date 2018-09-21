#pragma once
#include <GameExecutor/Executor/WebLink.h>

namespace P1 {
  namespace GameExecutor {
    namespace Executor {

       /*!
        \class ExecutableFile
        \brief Позволяет запустить игру, работающую по схеме `https`. 

        \code
        QUrl url("https://protocol.one");
      
        Core::Service service;
        service.setUrl(url);

        WebLink executor;
        executor.execute();
        \endcode

        \sa Core::Service, QUrl, WebLink
      */
      class GAMEEXECUTOR_EXPORT WebLinkSsl : public WebLink
      {
        Q_OBJECT
      public:
          explicit WebLinkSsl(QObject *parent = 0);
          virtual ~WebLinkSsl();
      };
    }
  }
}