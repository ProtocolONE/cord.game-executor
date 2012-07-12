#include <GameExecutor/GameExecutorServiceWrapper.h>
using namespace GGS;
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

void GameExecutorServiceWrapper::canExecuteCompleted(const Core::Service &service)
{
  if (this->_canExecuteCompletedFunc)
    this->_canExecuteCompletedFunc(service);
}

void GameExecutorServiceWrapper::preExecuteCompleted(const Core::Service &service)
{
  if (this->_preExecuteCompletedFunc) 
    this->_preExecuteCompletedFunc(service);
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

  connect(this->_service, SIGNAL(canExecuteCompleted(const GGS::Core::Service)), 
          this, SLOT(canExecuteCompleted(const GGS::Core::Service)));

  connect(this->_service, SIGNAL(preExecuteCompleted(const GGS::Core::Service)), 
          this, SLOT(preExecuteCompleted(const GGS::Core::Service)));

  connect(this->_service, SIGNAL(started(const GGS::Core::Service)), 
          this, SLOT(started(const GGS::Core::Service)));

  connect(this->_service, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)), 
          this, SLOT(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)), Qt::QueuedConnection);
}





