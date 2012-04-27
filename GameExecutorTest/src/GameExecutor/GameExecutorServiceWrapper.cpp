#include <GameExecutor/GameExecutorServiceWrapper.h>

GameExecutorServiceWrapper::GameExecutorServiceWrapper(QObject *parent /*= 0*/) : QObject(parent)
{

}

GameExecutorServiceWrapper::GameExecutorServiceWrapper(GGS::GameExecutor::GameExecutorService *_service)
  : QObject(_service)
{
  this->setGameExecutorService(_service);
}

GameExecutorServiceWrapper::~GameExecutorServiceWrapper()
{

}

void GameExecutorServiceWrapper::canExecuteCompleted(const Core::Service &service, bool result)
{
  if (this->_canExecuteCompletedFunc)
    this->_canExecuteCompletedFunc(service, result);
}

void GameExecutorServiceWrapper::preExecuteCompleted(const Core::Service &service, bool result)
{
  if (this->_preExecuteCompletedFunc) 
    this->_preExecuteCompletedFunc(service, result);
}

void GameExecutorServiceWrapper::started(const Core::Service &service)
{
  if (this->_startedFunc) 
    this->_startedFunc(service);
}

void GameExecutorServiceWrapper::finished(const Core::Service &service, GGS::GameExecutor::FinishState state)
{
  if (this->_finishedFunc)
    this->_finishedFunc(service, state);
}

void GameExecutorServiceWrapper::setCanExecuteCompleted(CanPreFunc func)
{
  this->_canExecuteCompletedFunc = func;
}

void GameExecutorServiceWrapper::setPreExecuteCompleted(CanPreFunc func)
{
  this->_preExecuteCompletedFunc = func;
}

void GameExecutorServiceWrapper::setStarted(StartedFunc func)
{
  this->_startedFunc = func;
}

void GameExecutorServiceWrapper::setFinished(FinishedFunc func)
{
  this->_finishedFunc = func;
}

void GameExecutorServiceWrapper::setGameExecutorService(GGS::GameExecutor::GameExecutorService *_service)
{
  this->_service = _service;

  connect(this->_service, SIGNAL(canExecuteCompleted(const Core::Service, bool)), 
          this, SLOT(canExecuteCompleted(const Core::Service, bool)));

  connect(this->_service, SIGNAL(preExecuteCompleted(const Core::Service, bool)), 
          this, SLOT(preExecuteCompleted(const Core::Service, bool)));

  connect(this->_service, SIGNAL(started(const Core::Service)), 
          this, SLOT(started(const Core::Service)));

  connect(this->_service, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)), 
          this, SLOT(finished(const Core::Service, GGS::GameExecutor::FinishState)), Qt::QueuedConnection);
}





