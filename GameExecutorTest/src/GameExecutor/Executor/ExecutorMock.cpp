#include <GameExecutor/Executor/ExecutorMock.h>

ExecutorMock::ExecutorMock(QObject *parent) : ExecutorBase(parent) 
{
  this->_scheme = QString("test");
}

ExecutorMock::~ExecutorMock()
{
}

void ExecutorMock::execute(
  const P1::Core::Service &service, 
  GameExecutorService *executorService,
  const P1::RestApi::ProtocolOneCredential& credential)
{
  this->credential = credential;
  this->secondCredential = secondCredential;
  emit this->started(service);
  emit this->finished(service, P1::GameExecutor::Success);
}
