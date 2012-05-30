#include <GameExecutor/hookinterface.h>

namespace GGS {
  namespace GameExecutor {

    HookInterface::HookInterface(QObject *parent /*= 0*/)
    {
    }

    HookInterface::~HookInterface()
    {
    }

    void HookInterface::CanExecute(const Core::Service &service)
    {
      emit this->canExecuteCompleted(true);
    }

    void HookInterface::PreExecute(const Core::Service &service)
    {
      emit this->preExecuteCompleted(true);
    }

    void HookInterface::PostExecute(const Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      emit this->postExecuteCompleted();
    }
  }
}
