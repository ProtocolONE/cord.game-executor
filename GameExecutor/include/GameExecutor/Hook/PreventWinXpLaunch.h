#pragma once

#include <Core/Service.h>
#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

namespace P1 {
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