/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_HOOK_DISABLEIEDEFALUTPROXY_H
#define _GGS_GAMEEXECUTOR_HOOK_DISABLEIEDEFALUTPROXY_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtCore/QObject>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      /*!
        \class DisableIEDefalutProxy
      
        \brief Отключение прокси перед запуском игры. Актуально для Aika2, которая на ряде ПК не запускается, если 
        в InternetExplorer выставлена любая прокси.
      */
      class GAMEEXECUTOR_EXPORT DisableIEDefalutProxy : public HookInterface
      {
        Q_OBJECT
      public:
        explicit DisableIEDefalutProxy(QObject *parent = 0);
        virtual ~DisableIEDefalutProxy();

        static QString id();

        virtual void PreExecute(Core::Service &service);
      };
    }
  }
}
#endif // _GGS_GAMEEXECUTOR_HOOK_DISABLEIEDEFALUTPROXY_H
