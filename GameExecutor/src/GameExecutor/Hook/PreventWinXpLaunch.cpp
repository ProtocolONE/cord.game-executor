/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/Hook/PreventWinXpLaunch.h>

#include <Core/UI/Message>

#include <QtCore/QSysInfo>

namespace GGS {
  namespace GameExecutor{
    namespace Hook{
      PreventWinXpLaunch::PreventWinXpLaunch(QObject *parent)
        : HookInterface(parent)
      {
      }

      PreventWinXpLaunch::~PreventWinXpLaunch()
      {
      }

      QString PreventWinXpLaunch::id()
      {
         return "153884F8-6E51-4890-91C2-A6316AC04C37";
      }

      void PreventWinXpLaunch::CanExecute(Core::Service &service)
      {
        if (QSysInfo::WV_XP == QSysInfo::WindowsVersion) {
          Core::UI::Message::warning(
            QObject::tr("TITLE_ATTENTION"), 
            QObject::tr("PREVENT_XP_LAUNCH_WARNING").arg(service.displayName())
          );

          emit this->canExecuteCompleted(service, GGS::GameExecutor::CanExecutionHookBreak);
          return;
        }

        emit this->canExecuteCompleted(service, GGS::GameExecutor::Success);
      }
   }
  }
}

