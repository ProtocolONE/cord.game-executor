/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_HOOK_DISABLEDEP_H
#define _GGS_GAMEEXECUTOR_HOOK_DISABLEDEP_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QObject>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      class GAMEEXECUTOR_EXPORT DisableDEP : public HookInterface
      {
        Q_OBJECT
      public:
        explicit DisableDEP(QObject *parent = 0);
        ~DisableDEP();

        virtual void PreExecute(const Core::Service &service);
      };
    }
  }
}
#endif // _GGS_GAMEEXECUTOR_HOOK_DISABLEDEP_H
