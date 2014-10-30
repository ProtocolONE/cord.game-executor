#ifndef EXECUTIONLOOP_H
#define EXECUTIONLOOP_H

#include <GameExecutor/Enum.h>

#include <Core/Service>

#include <RestApi/RestApiManager>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointer>

namespace GGS {
  namespace GameExecutor {

    class HookInterface;
    class ExecutorBase;
    class GameExecutorService;

    class ExecutionLoopPrivate : public QObject
    {
      Q_OBJECT
    public:
      ExecutionLoopPrivate(QObject *parent);
      ~ExecutionLoopPrivate();

      void setService(const GGS::Core::Service &val);
      void setHookList(QList<HookInterface*> &val);
      void setExecutorService(GGS::GameExecutor::GameExecutorService *val);
      void setExecutor(GGS::GameExecutor::ExecutorBase *val);
      void setCredential(const GGS::RestApi::GameNetCredential& value);
      void setSecondCredential(const GGS::RestApi::GameNetCredential& value);
      
      void execute();

    public slots:
      void onStopExecution();

    signals:
      void canExecuteCompleted(const GGS::Core::Service &service);
      void preExecuteCompleted(const GGS::Core::Service &service);
      void started(const GGS::Core::Service &service);
      void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);

    private slots:
      void executeHookCanStep(const GGS::Core::Service &serivce, GGS::GameExecutor::FinishState result);
      void executeHookPreStep(const GGS::Core::Service &service, GGS::GameExecutor::FinishState result);
      void executorStep();
      void executorCompletedStep(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);
      void executeHookPostStep(const GGS::Core::Service &service);

      void startedStep(const GGS::Core::Service &service);
  
    private:
      GGS::Core::Service _service;
      QList<HookInterface*> _list;
      quint32 _listIndex;
      QPointer<GGS::GameExecutor::GameExecutorService> _executorService;
      QPointer<GGS::GameExecutor::ExecutorBase> _executor;
      GGS::GameExecutor::FinishState _state;
      GGS::RestApi::GameNetCredential _credential;
      GGS::RestApi::GameNetCredential _secondCredential;
      bool _stopExecution;
    };
  }
}

#endif // EXECUTIONLOOP_H
