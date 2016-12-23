#include <GameExecutor/ExecutorBase.h>

namespace GGS {
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

    GGS::GameExecutor::FinishState ExecutorBase::finishStateFromRestApiErrorCode(int errorCode)
    {
      FinishState state;

      switch(errorCode) {
      case RestApi::CommandBase::AuthorizationFailed:
      case RestApi::CommandBase::AccountNotExists:
      case RestApi::CommandBase::AuthorizationLimitExceed:
      case RestApi::CommandBase::UnknownAccountStatus:
        state = AuthorizationError;
        break;
      case RestApi::CommandBase::GuestExpired:
        state = GuestAccountExpired;
        break;
      case RestApi::CommandBase::ServiceAccountBlocked:
        state = ServiceAccountBlockedError;
        break;
      case RestApi::CommandBase::ServiceAuthorizationImpossible:
        state = ServiceAuthorizationImpossible;
        break;
      case RestApi::CommandBase::PakkanenPermissionDenied:
        state = PakkanenPermissionDenied;
        break;
      case RestApi::CommandBase::PakkanenPhoneVerification:
      case RestApi::CommandBase::PakkanenVkVerification:
      case RestApi::CommandBase::PakkanenVkPhoneVerification:
        state = PakkanenPhoneVerification;
        break;
      case RestApi::CommandBase::PakkanenGeoIpBlocked:
        state = PakkanenGeoIpBlocked;
        break;

      default:
        state = UnhandledRestApiError;
      }

      return state;
    }

  }
};