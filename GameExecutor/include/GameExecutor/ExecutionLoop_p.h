#pragma once

#include <GameExecutor/Enum.h>

#include <Core/Service.h>

#include <RestApi/RestApiManager.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointer>

namespace P1 {
  namespace GameExecutor {

    class HookInterface;
    class ExecutorBase;
    class GameExecutorService;

    class ExecutionLoopPrivate : public QObject
    {
      Q_OBJECT
    public:
      explicit ExecutionLoopPrivate(QObject *parent);
      virtual ~ExecutionLoopPrivate();

      void setService(const P1::Core::Service &val);
      void setHookList(QList<HookInterface*> &val);
      void setExecutorService(P1::GameExecutor::GameExecutorService *val);
      void setExecutor(P1::GameExecutor::ExecutorBase *val);
      void setCredential(const P1::RestApi::ProtocolOneCredential& value);
      
      void execute();

    public slots:
      void onStopExecution();

    signals:
      void canExecuteCompleted(const P1::Core::Service &service);
      void preExecuteCompleted(const P1::Core::Service &service);
      void started(const P1::Core::Service &service);
      void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);

    private slots:
      void executeHookCanStep(const P1::Core::Service &serivce, P1::GameExecutor::FinishState result);
      void executeHookPreStep(const P1::Core::Service &service, P1::GameExecutor::FinishState result);
      void executorStep();
      void executorCompletedStep(const P1::Core::Service &service, P1::GameExecutor::FinishState state);
      void executeHookPostStep(const P1::Core::Service &service);

      void startedStep(const P1::Core::Service &service);
  
    private:
      P1::Core::Service _service;
      QList<HookInterface*> _list;
      quint32 _listIndex;
      QPointer<P1::GameExecutor::GameExecutorService> _executorService;
      QPointer<P1::GameExecutor::ExecutorBase> _executor;
      P1::GameExecutor::FinishState _state;
      P1::RestApi::ProtocolOneCredential _credential;
      bool _stopExecution;
    };
  }
}

