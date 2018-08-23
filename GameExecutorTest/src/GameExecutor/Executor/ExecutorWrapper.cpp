#include <GameExecutor/Executor/ExecutorWrapper.h>

ExecutorWrapper::ExecutorWrapper(QObject *parent) : QObject(parent)
{

}

ExecutorWrapper::ExecutorWrapper(P1::GameExecutor::ExecutorBase *executor) : QObject(executor)
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

void ExecutorWrapper::setExecutor(P1::GameExecutor::ExecutorBase *executor)
{
  connect(executor, SIGNAL(started(const P1::Core::Service)), this, SLOT(started(const P1::Core::Service)));
  connect(executor, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)), 
          this, SLOT(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));
}

void ExecutorWrapper::started(const Core::Service &service)
{
  if (this->_startedFunc)
    this->_startedFunc(service);
}

void ExecutorWrapper::finished(const Core::Service &service, P1::GameExecutor::FinishState state)
{
  if (this->_finishedFunc)
    this->_finishedFunc(service, state);
}