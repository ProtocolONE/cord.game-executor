#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QObject>

namespace P1 {
  namespace GameExecutor {
    namespace Hook {
      class GAMEEXECUTOR_EXPORT DisableDEP : public HookInterface
      {
        Q_OBJECT
      public:
        explicit DisableDEP(QObject *parent = 0);
        virtual ~DisableDEP();

        static QString id();

        virtual void PreExecute(Core::Service &service);
      };
    }
  }
}
