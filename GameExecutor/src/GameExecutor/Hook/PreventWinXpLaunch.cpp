#include <GameExecutor/Hook/PreventWinXpLaunch.h>

#include <Core/UI/Message.h>

#include <QtCore/QSysInfo>

namespace P1 {
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

          emit this->canExecuteCompleted(service, P1::GameExecutor::CanExecutionHookBreak);
          return;
        }

        emit this->canExecuteCompleted(service, P1::GameExecutor::Success);
      }
   }
  }
}

