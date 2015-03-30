/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (C) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#pragma once

#include <Core/Service>
#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      /*!
        \class PreventWinXpLaunch
      
        \brief Не даёт запустить игру на win xp. 
      */
      class GAMEEXECUTOR_EXPORT PreventWinXpLaunch : public HookInterface
      {
        Q_OBJECT

      public:
        explicit PreventWinXpLaunch(QObject *parent = 0);
        virtual ~PreventWinXpLaunch();

        static QString id();
        virtual void CanExecute(Core::Service &service);
      };
    }
  }
}