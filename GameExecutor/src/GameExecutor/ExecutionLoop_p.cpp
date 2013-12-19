/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameExecutor/ExecutionLoop_p.h>

#include <GameExecutor/HookInterface.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/GameExecutorService.h>

#include <Core/Marketing.h>
using GGS::Core::Marketing;

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
        SIGNAL_CONNECT_CHECK(connect(hook, SIGNAL(canExecuteCompleted(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), 
          this, SLOT(executeHookCanStep(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(connect(hook, SIGNAL(preExecuteCompleted(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), 
          this, SLOT(executeHookPreStep(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(connect(hook, SIGNAL(postExecuteCompleted(const GGS::Core::Service &)), 
          this, SLOT(executeHookPostStep(const GGS::Core::Service &)), Qt::QueuedConnection));
      }

      SIGNAL_CONNECT_CHECK(connect(this->_executor.data(), SIGNAL(started(const GGS::Core::Service)), 
        this, SLOT(startedStep(const GGS::Core::Service)), Qt::QueuedConnection));

      SIGNAL_CONNECT_CHECK(connect(this->_executor.data(), 
        SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)), 
        this, 
        SLOT(executorCompletedStep(const GGS::Core::Service, GGS::GameExecutor::FinishState)), 
        Qt::QueuedConnection));

      this->executeHookCanStep(this->_service, GGS::GameExecutor::Success);
    }

    void ExecutionLoopPrivate::executeHookCanStep(const GGS::Core::Service &service, GGS::GameExecutor::FinishState result) 
    {
      if (this->_service.id() != service.id())
        return;

      if (GGS::GameExecutor::Success != result) {
        emit this->finished(this->_service, result);
        return;
      }

      if (this->_listIndex == this->_list.size()) {
        emit this->canExecuteCompleted(this->_service);

        this->_listIndex = 0;
        QMetaObject::invokeMethod(this, "executeHookPreStep",  Qt::QueuedConnection, 
          Q_ARG(const GGS::Core::Service&, this->_service),
          Q_ARG(GGS::GameExecutor::FinishState, GGS::GameExecutor::Success));

        return;
      }

      HookInterface *hook = this->_list[this->_listIndex++];
      hook->CanExecute(this->_service);
    }

    void ExecutionLoopPrivate::executeHookPreStep(const GGS::Core::Service &service, GGS::GameExecutor::FinishState result) 
    {
      if (this->_service.id() != service.id())
        return;

      if (GGS::GameExecutor::Success != result) {
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
      Marketing::sendOnceByService(Marketing::FirstRunService, this->_service.id());
      Marketing::send(Marketing::StartService, this->_service.id());
      this->_executor->execute(this->_service, this->_executorService, this->_credential);
    }

    void ExecutionLoopPrivate::executorCompletedStep(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      if (this->_service.id() != service.id())
        return;

      this->_listIndex = 0;
      this->_state = state;
      this->_service = service;
      this->executeHookPostStep(this->_service);
    }

    void ExecutionLoopPrivate::executeHookPostStep(const GGS::Core::Service &service)
    {
      if (this->_service.id() != service.id())
        return;

      if (this->_listIndex == this->_list.size()) {
        Marketing::send(Marketing::CloseService, this->_service.id());
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

    void ExecutionLoopPrivate::startedStep(const GGS::Core::Service &service)
    {
      if (this->_service.id() != service.id())
        return;

      emit this->started(service);
    }

    void ExecutionLoopPrivate::setCredential(const GGS::RestApi::GameNetCredential& value)
    {
      this->_credential = value;
    }

  }
}