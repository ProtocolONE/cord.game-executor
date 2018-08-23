#pragma once
#include <GameExecutor/ExecutorBase.h>

#include <Core/Service.h>

#include <functional>
#include <QObject>

using namespace P1;

typedef std::function<void(const Core::Service &)> ExecutorStartedFunc;
typedef std::function<void(const Core::Service &, P1::GameExecutor::FinishState)> ExecutorFinishedFunc;

class ExecutorWrapper : public QObject
{
  Q_OBJECT

public:
  ExecutorWrapper(QObject *parent = 0);
  ExecutorWrapper(P1::GameExecutor::ExecutorBase *executor);
  ~ExecutorWrapper();

  void setStarted(ExecutorStartedFunc func);
  void setFinished(ExecutorFinishedFunc func);
  void setExecutor(P1::GameExecutor::ExecutorBase *executor);

private slots:
  void started(const P1::Core::Service &service);
  void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);

private:
  ExecutorStartedFunc _startedFunc;
  ExecutorFinishedFunc _finishedFunc;
};

