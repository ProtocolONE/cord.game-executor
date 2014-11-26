/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2014, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

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

namespace GGS {
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