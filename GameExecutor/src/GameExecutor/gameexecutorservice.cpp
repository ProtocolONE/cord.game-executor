#include <GameExecutor/gameexecutorservice.h>

#include <QtCore/QtConcurrentRun>

namespace GGS {
  namespace GameExecutor {

    GameExecutorService::GameExecutorService(QObject *parent) : QObject(parent)
    {
    }

    GameExecutorService::~GameExecutorService()
    {
    }

    void GameExecutorService::setRestApiManager(RestApi::RestApiManager *restApiManager )
    {
      this->_restApiManager = restApiManager; 
    }

    RestApi::RestApiManager *GameExecutorService::respApiManager()
    {
      return this->_restApiManager; 
    }

    bool GameExecutorService::registerExecutor(ExecutorBase *executor)
    {
      QString scheme = executor->scheme();
      if (this->hasExecutor(scheme)) {
        return false;
      }

      this->_executors[scheme] = executor;

      connect(executor, SIGNAL(started(const Core::Service)), 
              this, SIGNAL(started(const Core::Service)), Qt::QueuedConnection);

      connect(executor, 
              SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)), 
              this, 
              SLOT(internalExecutionFinished(const Core::Service, GGS::GameExecutor::FinishState)), 
              Qt::QueuedConnection);
      
      return true;
    }

    bool GameExecutorService::hasExecutor(const QString &scheme) const
    {
      return this->_executors.contains(scheme);
    }

    bool GameExecutorService::addHook(const Core::Service &service, HookInterface *hook, int priority /*= 0*/)
    {
      QString id = service.id();
      if (id.isEmpty()) {
        return false;
      }

      this->_hooks[id].insert(priority, hook);

      return true;
    }

    void GameExecutorService::clearHooks(const Core::Service &service)
    {
      QString id = service.id();
      if (id.isEmpty()) {
        return;
      }
      
      this->_hooks[id].clear();
    }

    void GameExecutorService::execute(const Core::Service &service)
    {
      QString id = service.id();
      if (id.isEmpty()) {
        emit this->finished(service, InvalidService);
        return;
      }

      QUrl url = service.url();
      QString scheme = url.scheme();

      if (!this->hasExecutor(scheme)) {
        emit this->finished(service, UnknownSchemeError);
        return;
      }

      QtConcurrent::run(this, &GameExecutorService::internalExecutionLoop, service);
    }

    void GameExecutorService::internalExecutionLoop(const Core::Service &service )
    {
      bool result = this->internalCanExecuteLoop(service);
      if (!result) {
        emit this->finished(service, GGS::GameExecutor::CanExecutionHookBreak);
        return;
      }

      emit this->canExecuteCompleted(service, result);

      result = this->internalPreExecuteLoop(service);
      if (!result) {
        emit this->finished(service, GGS::GameExecutor::PreExecutionHookBreak);
        return;
      }

      emit this->preExecuteCompleted(service, result);

      this->internalExecuteLoop(service);
    }

    bool GameExecutorService::internalCanExecuteLoop(const Core::Service &service)
    {
      QString id = service.id();

      if (!this->_hooks.contains(id)) {
        return true;
      }

      Q_FOREACH (HookInterface *hook, this->_hooks[id]) {
        if (false == hook->CanExecute(service)) {
          return false;
        };
      }

      return true;
    }

    bool GameExecutorService::internalPreExecuteLoop(const Core::Service &service)
    {
      QString id = service.id();

      if (!this->_hooks.contains(id)) {
        return true;
      }

      Q_FOREACH (HookInterface *hook, this->_hooks[id]) {
        if (false == hook->PreExecute(service)) {
          return false;
        };
      }

      return true;
    }

    void GameExecutorService::internalExecuteLoop(const Core::Service &service)
    {
      QString scheme = service.url().scheme();
      this->_executors[scheme]->execute(service, this);
    }

    void GameExecutorService::internalExecutionFinished(const Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      QtConcurrent::run(this, &GameExecutorService::internalPostExecuteLoop, service, state);
    }

    void GameExecutorService::internalPostExecuteLoop(const Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      QString id = service.id();
    
      Q_FOREACH (HookInterface *hook, this->_hooks[id]) {
        hook->PostExecute(service, state);
      }

      emit this->finished(service, state);
    }
  }
}