/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (c) 2011 - 2015, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameExecutor/Executor/WebLink.h>

namespace GGS {
  namespace GameExecutor {
    namespace Executor {

       /*!
        \class ExecutableFile
        \brief Позволяет запустить игру, работающую по схеме `https`. 

        \code
        QUrl url("https://gamenet.ru");
      
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