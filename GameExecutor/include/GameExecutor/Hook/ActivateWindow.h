#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtWidgets/QWidget>

/*
  QGNA-1058
  Хук создает скрытое окно, которое нужно активировать каждый раз при запуске игры. 

  Т.к в новом qgna, хост процесс не имеет окна, для windows 7 и ниже, при создании нового процесса, 
  если у вызываемого процесса нет окна, и оно не в фокусе, фокус создаваемому процессу не будет передан. 
  Этот хук предназначен что-бы исправить это это поведение.
*/

namespace P1 {
  namespace GameExecutor {
    namespace Hook {

      class GAMEEXECUTOR_EXPORT ActivateWindow : public HookInterface
      {
        Q_OBJECT
      public:
        explicit ActivateWindow(QObject *parent = 0);
        virtual ~ActivateWindow();

        virtual void PreExecute(Core::Service &service);
        void setWidget(QWidget* widget);

        static QString id();

        private:
          QWidget* _window;
      };

    }
  }
}