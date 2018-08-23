#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtCore/QObject>

namespace P1 {
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
