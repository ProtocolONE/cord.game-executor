#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QObject>
#include <windows.h>

namespace P1 {
  namespace GameExecutor {
    namespace Hook {
      /*!
        \class RestoreResolution
      
        \brief Сохраняет и восстанавливает разрешение экрана после запуска игры. 
      */
      class GAMEEXECUTOR_EXPORT RestoreResolution : public HookInterface
      {
        Q_OBJECT
      public:
        explicit RestoreResolution(QObject *parent = 0);
        virtual ~RestoreResolution();

        static QString id();

        virtual void PostExecute(Core::Service &service, P1::GameExecutor::FinishState state);
        virtual void PreExecute(Core::Service &service);

      private:
        DEVMODE _beforeExecuteDisplay;
        bool _enabled;
      };
    }
  }
}

