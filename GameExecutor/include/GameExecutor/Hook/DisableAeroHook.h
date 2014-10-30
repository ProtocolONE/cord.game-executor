/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#ifndef _GGS_GAMEEXCUTOR_HOOK_DISABLEAEROHOOK_H_
#define _GGS_GAMEEXCUTOR_HOOK_DISABLEAEROHOOK_H_

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

namespace GGS {
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

#endif // _GGS_GAMEEXCUTOR_HOOK_DISABLEAEROHOOK_H_