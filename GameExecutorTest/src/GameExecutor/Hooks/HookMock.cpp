#include <GameExecutor/Hooks/HookMock.h>

HookMock::HookMock(QObject *parent) 
  : P1::GameExecutor::HookInterface(parent)
{
}

HookMock::~HookMock()
{
}

void HookMock::CanExecute(Core::Service &service)
{
  P1::GameExecutor::FinishState ret = P1::GameExecutor::Success;
  if (this->_canFunc) {
    ret = this->_canFunc(service);
  }

  emit this->canExecuteCompleted(service, ret);
}

void HookMock::PreExecute(Core::Service &service)
{
  P1::GameExecutor::FinishState ret = P1::GameExecutor::Success;
  if (this->_preFunc) {
    ret = this->_preFunc(service);
  }

  emit this->preExecuteCompleted(service, ret);
}

void HookMock::PostExecute(Core::Service &service, P1::GameExecutor::FinishState state)
{
  if (this->_postFunc) {
    this->_postFunc(service, state);
  }

  emit this->postExecuteCompleted(service);
}

void HookMock::setCanFunc(HookCanPreFunc func)
{
  this->_canFunc = func;
}

void HookMock::setPreFunc(HookCanPreFunc func)
{
  this->_preFunc = func;
}

void HookMock::setPostFunc(HookPostFunc func)
{
  this->_postFunc = func;
}
