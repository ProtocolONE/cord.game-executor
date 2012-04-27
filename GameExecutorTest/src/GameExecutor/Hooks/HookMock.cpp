#include <GameExecutor/Hooks/HookMock.h>

HookMock::HookMock(QObject *parent) : QObject(parent)
{

}

HookMock::~HookMock()
{

}

bool HookMock::CanExecute( const Core::Service &service )
{
  if (this->_canFunc) {
    return this->_canFunc(service);
  }

  return true;
}

bool HookMock::PreExecute( const Core::Service &service )
{
  if (this->_preFunc) {
    return this->_preFunc(service);
  }

  return true;
}

void HookMock::PostExecute( const Core::Service &service, GGS::GameExecutor::FinishState state )
{
  if (this->_postFunc) {
    this->_postFunc(service, state);
  }
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
