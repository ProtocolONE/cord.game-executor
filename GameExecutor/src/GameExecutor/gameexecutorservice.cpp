#include <GameExecutor/ExecutionLoop_p.h>
#include <GameExecutor/gameexecutorservice.h>

#include <QtCore/QList>

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

      QList<HookInterface *> list;
      Q_FOREACH (HookInterface *hook, this->_hooks[id]) {
        list.append(hook);
      }

      ExecutionLoopPrivate *loop = new ExecutionLoopPrivate(this);
      loop->setService(service);
      loop->setHookList(list);
      loop->setExecutor(this->_executors[scheme]);
      loop->setExecutorService(this);

      connect(loop, SIGNAL(canExecuteCompleted(const Core::Service &, bool)),
        this, SIGNAL(canExecuteCompleted(const Core::Service &, bool)), Qt::QueuedConnection);

      connect(loop, SIGNAL(preExecuteCompleted(const Core::Service &, bool)),
        this, SIGNAL(preExecuteCompleted(const Core::Service &, bool)), Qt::QueuedConnection);

      connect(loop, SIGNAL(started(const Core::Service &)),
        this, SIGNAL(started(const Core::Service &)), Qt::QueuedConnection);

      connect(loop, SIGNAL(finished(const Core::Service &, GGS::GameExecutor::FinishState)),
        this, SLOT(privateFinished(const Core::Service &, GGS::GameExecutor::FinishState)), Qt::QueuedConnection);

      loop->execute();
    }

    void GameExecutorService::privateFinished( const Core::Service &service, GGS::GameExecutor::FinishState state )
    {
      QObject::sender()->deleteLater();
      emit this->finished(service, state);
    }

  }
}