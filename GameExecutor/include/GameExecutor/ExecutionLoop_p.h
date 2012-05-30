#ifndef EXECUTIONLOOP_H
#define EXECUTIONLOOP_H

#include <GameExecutor/Enum.h>

#include <Core/Service>

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
      
      void execute();

    signals:
      void canExecuteCompleted(const Core::Service &service, bool result);
      void preExecuteCompleted(const Core::Service &service, bool result);
      void started(const Core::Service &service);
      void finished(const Core::Service &service, GGS::GameExecutor::FinishState state);

    private slots:
      void executeHookCanStep(bool result);
      void executeHookPreStep(bool result);
      void executorStep();
      void executorCompletedStep(const Core::Service &service, GGS::GameExecutor::FinishState state);
      void executeHookPostStep();
  
    private:
      GGS::Core::Service _service;
      QList<HookInterface*> _list;
      quint32 _listIndex;
      QPointer<GGS::GameExecutor::GameExecutorService> _executorService;
      QPointer<GGS::GameExecutor::ExecutorBase> _executor;
      GGS::GameExecutor::FinishState _state;
    };
  }
}

#endif // EXECUTIONLOOP_H
