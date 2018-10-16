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
      UnhandledRestApiError,
      CanExecutionHookBreak,
      PreExecutionHookBreak,
      AlreadyStartedError,
    };

    Q_ENUMS(P1::GameExecutor::FinishState);
  }
}

Q_DECLARE_METATYPE(P1::GameExecutor::FinishState);
