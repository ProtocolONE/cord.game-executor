/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_ENUM_H
#define _GGS_GAMEEXECUTOR_ENUM_H

#include <QMetaType>

namespace GGS {
  namespace GameExecutor {
    enum FinishState {
      Success = 0, 
      InvalidService,
      UnknownSchemeError, 
      InternalFatalError, 
      ExternalFatalError, 
      AuthorizationError = 5, 
      GuestAccountExpired = 6,
      UnhandledRestApiError,
      CanExecutionHookBreak,
      PreExecutionHookBreak,
      AlreadyStartedError,
      ServiceAccountBlockedError = 102,
      ServiceAuthorizationImpossible = 125,
      PakkanenPermissionDenied = 601,
      PakkanenPhoneVerification = 603,
    };

    Q_ENUMS(GGS::GameExecutor::FinishState);
  }
}

Q_DECLARE_METATYPE(GGS::GameExecutor::FinishState);

#endif //_GGS_GAMEEXECUTOR_ENUM_H
