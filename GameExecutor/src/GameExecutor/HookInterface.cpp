#include <GameExecutor/hookinterface.h>

namespace P1 {
  namespace GameExecutor {

    HookInterface::HookInterface(QObject *parent /*= 0*/)
    {
    }

    HookInterface::~HookInterface()
    {
    }

    void HookInterface::CanExecute(Core::Service &service)
    {
      emit this->canExecuteCompleted(service, P1::GameExecutor::Success);
    }

    void HookInterface::PreExecute(Core::Service &service)
    {
      emit this->preExecuteCompleted(service, P1::GameExecutor::Success);
    }

    void HookInterface::PostExecute(Core::Service &service, P1::GameExecutor::FinishState state)
    {
      emit this->postExecuteCompleted(service);
    }

    void HookInterface::setCredential(const P1::RestApi::ProtocolOneCredential& value)
    {
      this->_credential = value;
    }

  }
}
