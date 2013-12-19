/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_EXECUTOR_WEBLINK_H
#define _GGS_GAMEEXECUTOR_EXECUTOR_WEBLINK_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>

#include <QObject>

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

      /*!
      \class ExecutableFile
      \brief Позволяет запустить игру, работающую по схеме `exe`

      Позволяет запустить игру, работающую по схеме `http`. 

      \code
      QUrl url("http://www.playga.ru");
      
      Core::Service service;
      service.setUrl(url);

      WebLink executor;
      executor.execute();
      \endcode

      \sa Core::Service, QUrl
      */
      class GAMEEXECUTOR_EXPORT WebLink : public GGS::GameExecutor::ExecutorBase
      {
        Q_OBJECT
      public:
        explicit WebLink(QObject *parent = 0);
        ~WebLink();

        virtual void execute(
          const GGS::Core::Service &service, 
          GameExecutorService *executorService,
          const GGS::RestApi::GameNetCredential& credential) override;

      };
    }
  }
}

#endif // _GGS_GAMEEXECUTOR_EXECUTOR_WEBLINK_H
