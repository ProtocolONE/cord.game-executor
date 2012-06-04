#include <GameExecutor/ExecutionLoop_p.h>

#include <GameExecutor/HookInterface.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/GameExecutorService.h>

namespace GGS {
  namespace GameExecutor {
    ExecutionLoopPrivate::ExecutionLoopPrivate(QObject *parent)
      : QObject(parent),
      _listIndex(0)
    {

    }

    ExecutionLoopPrivate::~ExecutionLoopPrivate()
    {
    }

    void ExecutionLoopPrivate::execute()
    {
      this->_listIndex = 0;

      Q_FOREACH(HookInterface *hook, this->_list) {
        connect(hook, SIGNAL(canExecuteCompleted(bool)), this, SLOT(executeHookCanStep(bool)), Qt::QueuedConnection);
        connect(hook, SIGNAL(preExecuteCompleted(bool)), this, SLOT(executeHookPreStep(bool)), Qt::QueuedConnection);
        connect(hook, SIGNAL(postExecuteCompleted()), 
                this, SLOT(executeHookPostStep()), Qt::QueuedConnection);
      }

      connect(this->_executor.data(), SIGNAL(started(const Core::Service)), 
              this, SIGNAL(started(const Core::Service)), Qt::QueuedConnection);

      connect(this->_executor.data(), 
              SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)), 
              this, 
              SLOT(executorCompletedStep(const Core::Service, GGS::GameExecutor::FinishState)), 
              Qt::QueuedConnection);

      this->executeHookCanStep(true);
    }

    void ExecutionLoopPrivate::executeHookCanStep(bool result) 
    {
      if (!result) {
        emit this->finished(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
        return;
      }

      if (this->_listIndex == this->_list.size()) {
        emit this->canExecuteCompleted(this->_service, true);

        this->_listIndex = 0;
        QMetaObject::invokeMethod(this, "executeHookPreStep",  Qt::QueuedConnection, Q_ARG(bool, true));
        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->CanExecute(this->_service);
    }

    void ExecutionLoopPrivate::executeHookPreStep(bool result) 
    {
      if (!result) {
        emit this->finished(this->_service, GGS::GameExecutor::PreExecutionHookBreak);
        return;
      }

      if (this->_listIndex == this->_list.size()) {
        emit this->preExecuteCompleted(this->_service, true);
        this->_listIndex = 0;

        QMetaObject::invokeMethod(this, "executorStep", Qt::QueuedConnection);
        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->PreExecute(this->_service);
    }

    void ExecutionLoopPrivate::executorStep()
    {
      this->_executor->execute(this->_service, this->_executorService);
    }

    void ExecutionLoopPrivate::executorCompletedStep(const Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      this->_listIndex = 0;
      this->_state = state;
      this->executeHookPostStep();
    }

    void ExecutionLoopPrivate::executeHookPostStep()
    {
      if (this->_listIndex == this->_list.size()) {
        emit this->finished(this->_service, this->_state);
        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->PostExecute(this->_service, this->_state);
    }

    void ExecutionLoopPrivate::setService(const GGS::Core::Service &val)
    {
      this->_service = val;
    }

    void ExecutionLoopPrivate::setHookList(QList<HookInterface*> &val)
    {
      this->_list = val;
    }

    void ExecutionLoopPrivate::setExecutorService(GGS::GameExecutor::GameExecutorService *val)
    {
      this->_executorService = val;
    }

    void ExecutionLoopPrivate::setExecutor(GGS::GameExecutor::ExecutorBase *val)
    {
      this->_executor = val;
    }
  }
}