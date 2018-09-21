#include <GameExecutor/ExecutionLoop_p.h>

#include <GameExecutor/HookInterface.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/GameExecutorService.h>

namespace P1 {
  namespace GameExecutor {
    ExecutionLoopPrivate::ExecutionLoopPrivate(QObject *parent)
      : QObject(parent)
      , _listIndex(0)
      , _stopExecution(false)
    {
    }

    ExecutionLoopPrivate::~ExecutionLoopPrivate()
    {
    }

    void ExecutionLoopPrivate::execute()
    {
      this->_listIndex = 0;

      Q_FOREACH(HookInterface *hook, this->_list) {
        hook->setCredential(this->_credential);

        QObject::connect(hook, &HookInterface::canExecuteCompleted,
          this, &ExecutionLoopPrivate::executeHookCanStep, Qt::QueuedConnection);

        QObject::connect(hook, &HookInterface::preExecuteCompleted,
          this, &ExecutionLoopPrivate::executeHookPreStep, Qt::QueuedConnection);

        QObject::connect(hook, &HookInterface::postExecuteCompleted,
          this, &ExecutionLoopPrivate::executeHookPostStep, Qt::QueuedConnection);
      }

      QObject::connect(this->_executor.data(), &ExecutorBase::started,
        this, &ExecutionLoopPrivate::startedStep, Qt::QueuedConnection);

      QObject::connect(this->_executor.data(), &ExecutorBase::finished,
        this, &ExecutionLoopPrivate::executorCompletedStep, Qt::QueuedConnection);

      this->executeHookCanStep(this->_service, P1::GameExecutor::Success);
    }

    void ExecutionLoopPrivate::executeHookCanStep(const P1::Core::Service &service, P1::GameExecutor::FinishState result) 
    {
      if (this->_service.id() != service.id())
        return;

      if (P1::GameExecutor::Success != result) {
        emit this->finished(this->_service, result);
        return;
      }

      if (this->_listIndex == this->_list.size()) {
        emit this->canExecuteCompleted(this->_service);

        this->_listIndex = 0;
        QMetaObject::invokeMethod(this, "executeHookPreStep",  Qt::QueuedConnection, 
          Q_ARG(const P1::Core::Service&, this->_service),
          Q_ARG(P1::GameExecutor::FinishState, P1::GameExecutor::Success));

        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->CanExecute(this->_service);
    }

    void ExecutionLoopPrivate::executeHookPreStep(const P1::Core::Service &service, P1::GameExecutor::FinishState result) 
    {
      if (this->_service.id() != service.id())
        return;

      if (P1::GameExecutor::Success != result) {
        emit this->finished(this->_service, result);
        return;
      }

      if (this->_listIndex == this->_list.size()) {
        emit this->preExecuteCompleted(this->_service);
        this->_listIndex = 0;

        QMetaObject::invokeMethod(this, "executorStep", Qt::QueuedConnection);
        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->PreExecute(this->_service);
    }

    void ExecutionLoopPrivate::executorStep()
    {
      if (this->_stopExecution) {
        emit this->finished(this->_service, P1::GameExecutor::Success);
      } else {
        this->_executor->execute(this->_service, this->_executorService, this->_credential);
      }
    }

    void ExecutionLoopPrivate::executorCompletedStep(const P1::Core::Service &service, P1::GameExecutor::FinishState state)
    {
      if (this->_service.id() != service.id())
        return;

      this->_listIndex = 0;
      this->_state = state;
      this->_service = service;
      this->executeHookPostStep(this->_service);
    }

    void ExecutionLoopPrivate::executeHookPostStep(const P1::Core::Service &service)
    {
      if (this->_service.id() != service.id())
        return;

      if (this->_listIndex == this->_list.size()) {
        emit this->finished(this->_service, this->_state);
        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->PostExecute(this->_service, this->_state);
    }

    void ExecutionLoopPrivate::setService(const P1::Core::Service &val)
    {
      this->_service = val;
    }

    void ExecutionLoopPrivate::setHookList(QList<HookInterface*> &val)
    {
      this->_list = val;
    }

    void ExecutionLoopPrivate::setExecutorService(P1::GameExecutor::GameExecutorService *val)
    {
      this->_executorService = val;
    }

    void ExecutionLoopPrivate::setExecutor(P1::GameExecutor::ExecutorBase *val)
    {
      this->_executor = val;
    }

    void ExecutionLoopPrivate::startedStep(const P1::Core::Service &service)
    {
      if (this->_service.id() != service.id())
        return;

      emit this->started(service);
    }

    void ExecutionLoopPrivate::onStopExecution()
    {
      //  INFO: просто выставляем флаг, ничего не вызывая у Executor'a, т.к.
      //  он сам подписывается на соответствующий сигнал ExecutorService
      this->_stopExecution = true;
    }

    void ExecutionLoopPrivate::setCredential(const P1::RestApi::ProtocolOneCredential& value)
    {
      this->_credential = value;
    }
  }
}