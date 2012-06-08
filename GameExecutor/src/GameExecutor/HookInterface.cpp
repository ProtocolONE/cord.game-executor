#include <GameExecutor/hookinterface.h>

namespace GGS {
  namespace GameExecutor {

    HookInterface::HookInterface(QObject *parent /*= 0*/)
    {
    }

    HookInterface::~HookInterface()
    {
    }

    void HookInterface::CanExecute(Core::Service &service)
    {
      emit this->canExecuteCompleted(true);
    }

    void HookInterface::PreExecute(Core::Service &service)
    {
      emit this->preExecuteCompleted(true);
    }

    void HookInterface::PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      emit this->postExecuteCompleted();
    }
  }
}
