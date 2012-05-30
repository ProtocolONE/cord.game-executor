#include <GameExecutor/Hooks/HookMock.h>

HookMock::HookMock(QObject *parent) 
  : GGS::GameExecutor::HookInterface(parent)
{
}

HookMock::~HookMock()
{
}

void HookMock::CanExecute(const Core::Service &service)
{
  bool ret = true;
  if (this->_canFunc) {
    ret = this->_canFunc(service);
  }

  emit this->canExecuteCompleted(ret);
}

void HookMock::PreExecute( const Core::Service &service )
{
  bool ret = true;
  if (this->_preFunc) {
    ret = this->_preFunc(service);
  }

  emit this->preExecuteCompleted(ret);
}

void HookMock::PostExecute( const Core::Service &service, GGS::GameExecutor::FinishState state )
{
  if (this->_postFunc) {
    this->_postFunc(service, state);
  }

  emit this->postExecuteCompleted();
}

void HookMock::setCanFunc( HookCanPreFunc func )
{
  this->_canFunc = func;
}

void HookMock::setPreFunc( HookCanPreFunc func )
{
  this->_preFunc = func;
}

void HookMock::setPostFunc( HookPostFunc func )
{
  this->_postFunc = func;
}
