/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_HOOK_RESTORERESOLUTION_H
#define _GGS_GAMEEXECUTOR_HOOK_RESTORERESOLUTION_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QObject>
#include <windows.h>

namespace GGS {
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
        ~RestoreResolution();

        virtual void PostExecute(const Core::Service &service, GGS::GameExecutor::FinishState state);

        virtual void PreExecute(const Core::Service &service);
      private:
        DEVMODE _beforeExecuteDisplay;
        bool _enabled;
      };
    }
  }
}


#endif // _GGS_GAMEEXECUTOR_HOOK_RESTORERESOLUTION_H
