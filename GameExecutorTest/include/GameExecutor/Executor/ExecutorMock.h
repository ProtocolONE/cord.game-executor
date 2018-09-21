#pragma once

#include <GameExecutor/ExecutorBase.h>

#include <RestApi/ProtocolOneCredential.h>

#include <Core/Service.h>
#include <QObject>

using namespace P1;
using namespace P1::GameExecutor;

class ExecutorMock : public ExecutorBase
{
  Q_OBJECT

public:
    ExecutorMock(QObject *parent = 0);
    ~ExecutorMock();

    virtual void execute(
      const P1::Core::Service &service, 
      GameExecutorService *executorService,
      const P1::RestApi::ProtocolOneCredential& credential) override;

    P1::RestApi::ProtocolOneCredential credential;
    P1::RestApi::ProtocolOneCredential secondCredential;
};
