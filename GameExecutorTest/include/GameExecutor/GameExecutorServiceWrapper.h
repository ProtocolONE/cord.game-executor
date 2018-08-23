#pragma once

#include <GameExecutor/GameExecutorService.h>

#include <functional>
#include <QObject>

typedef std::function<void(const P1::Core::Service &)> CanPreFunc;
typedef std::function<void(const P1::Core::Service &)> StartedFunc;
typedef std::function<void(const P1::Core::Service &, P1::GameExecutor::FinishState)> FinishedFunc;

class GameExecutorServiceWrapper : public QObject
{
  Q_OBJECT

public:
  explicit GameExecutorServiceWrapper(QObject *parent = 0);
  explicit GameExecutorServiceWrapper(P1::GameExecutor::GameExecutorService *_service) ;
  ~GameExecutorServiceWrapper();

  void setGameExecutorService(P1::GameExecutor::GameExecutorService *_service);

  void setCanExecuteCompleted(CanPreFunc func);
  void setPreExecuteCompleted(CanPreFunc func);
  void setStarted(StartedFunc func);
  void setFinished(FinishedFunc func);

private slots:
  void canExecuteCompleted(const P1::Core::Service &service);
  void preExecuteCompleted(const P1::Core::Service &service);
  void started(const P1::Core::Service &service);
  void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);

private:
  CanPreFunc _canExecuteCompletedFunc;
  CanPreFunc _preExecuteCompletedFunc;
  StartedFunc _startedFunc;
  FinishedFunc _finishedFunc;

  P1::GameExecutor::GameExecutorService *_service;

};
