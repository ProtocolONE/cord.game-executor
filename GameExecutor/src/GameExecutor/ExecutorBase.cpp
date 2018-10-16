#include <GameExecutor/ExecutorBase.h>

namespace P1 {
  namespace GameExecutor {
    ExecutorBase::ExecutorBase(QObject *parent /*= 0*/)
      : QObject(parent)
    {
    }

    ExecutorBase::ExecutorBase(const QString &scheme, QObject *parent /*= 0*/)
      : _scheme(scheme)
    {
    }

    ExecutorBase::~ExecutorBase()
    {
    }

    const QString &ExecutorBase::scheme() const
    {
      return this->_scheme;
    }

    P1::GameExecutor::FinishState ExecutorBase::finishStateFromRestApiErrorCode(int errorCode)
    {
      FinishState state;

      switch(errorCode) {
      case RestApi::Command::CommandBase::Unauthorized:
        state = AuthorizationError;
        break;
      case RestApi::Command::CommandBase::NetworkError:
      case RestApi::Command::CommandBase::BadFormat:
        state = ExternalFatalError;
        break;
      default:
        state = UnhandledRestApiError;
      }

      return state;
    }

  }
};