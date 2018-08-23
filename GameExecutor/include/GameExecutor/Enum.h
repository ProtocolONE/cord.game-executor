#pragma once

#include <QMetaType>

namespace P1 {
  namespace GameExecutor {
    enum FinishState : quint32 {
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
      PakkanenGeoIpBlocked = 605,
    };

    Q_ENUMS(P1::GameExecutor::FinishState);
  }
}

Q_DECLARE_METATYPE(P1::GameExecutor::FinishState);
