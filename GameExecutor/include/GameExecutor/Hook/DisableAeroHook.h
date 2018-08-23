#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

namespace P1 {
  namespace GameExecutor {
    namespace Hook {

      class GAMEEXECUTOR_EXPORT DisableAeroHook : public HookInterface
      {
      public:
        explicit DisableAeroHook(QObject *parent = 0);
        virtual ~DisableAeroHook();

        static QString id();

        virtual void CanExecute(Core::Service &service);

      private:
        void disableAero(const Core::Service &service);

      };

    }
  }
}
