#include <GameExecutor/Executor/ExecutorMock.h>

ExecutorMock::ExecutorMock(QObject *parent) : ExecutorBase(parent) 
{
  this->_scheme = QString("test");
}

ExecutorMock::~ExecutorMock()
{
}

void ExecutorMock::execute(
  const GGS::Core::Service &service, 
  GameExecutorService *executorService,
  const GGS::RestApi::GameNetCredential& credential,
  const GGS::RestApi::GameNetCredential& secondCredential)
{
  this->credential = credential;
  this->secondCredential = secondCredential;
  emit this->started(service);
  emit this->finished(service, GGS::GameExecutor::Success);
}
