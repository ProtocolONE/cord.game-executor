#include <GameExecutor/Executor/ExecutorWrapper.h>

ExecutorWrapper::ExecutorWrapper(QObject *parent) : QObject(parent)
{

}

ExecutorWrapper::ExecutorWrapper(GGS::GameExecutor::ExecutorBase *executor) : QObject(executor)
{
  this->setExecutor(executor);
}

ExecutorWrapper::~ExecutorWrapper()
{

}

void ExecutorWrapper::setStarted( ExecutorStartedFunc func )
{
  this->_startedFunc = func;
}

void ExecutorWrapper::setFinished( ExecutorFinishedFunc func )
{
  this->_finishedFunc = func;
}

void ExecutorWrapper::setExecutor(GGS::GameExecutor::ExecutorBase *executor)
{
  connect(executor, SIGNAL(started(const Core::Service)), this, SLOT(started(const Core::Service)));
  connect(executor, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)), 
          this, SLOT(finished(const Core::Service, GGS::GameExecutor::FinishState)));
}

void ExecutorWrapper::started(const Core::Service &service)
{
  if (this->_startedFunc)
    this->_startedFunc(service);
}

void ExecutorWrapper::finished(const Core::Service &service, GGS::GameExecutor::FinishState state)
{
  if (this->_finishedFunc)
    this->_finishedFunc(service, state);
}