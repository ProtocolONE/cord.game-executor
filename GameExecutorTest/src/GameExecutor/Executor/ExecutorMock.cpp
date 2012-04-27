#include <GameExecutor/Executor/ExecutorMock.h>

ExecutorMock::ExecutorMock(QObject *parent) : ExecutorBase(parent) 
{
  this->_scheme = QString("test");
}

ExecutorMock::~ExecutorMock()
{

}

void ExecutorMock::execute(const Core::Service &service, GameExecutorService *executorService)
{
  emit this->started(service);
  emit this->finished(service, GGS::GameExecutor::Success);
}
