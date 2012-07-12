#include <GameExecutor/ExecutionLoop_p.h>
#include <GameExecutor/gameexecutorservice.h>

#include <QtCore/QList>
#include <QtCore/QMutexLocker>

namespace GGS {
  namespace GameExecutor {

    GameExecutorService::GameExecutorService(QObject *parent) : QObject(parent)
    {
    }

    GameExecutorService::~GameExecutorService()
    {
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

    bool GameExecutorService::addHook(const GGS::Core::Service &service, HookInterface *hook, int priority /*= 0*/)
    {
      QString id = service.id();
      if (id.isEmpty()) {
        return false;
      }

      this->_hooks[id].insert(priority, hook);
      return true;
    }

    void GameExecutorService::clearHooks(const GGS::Core::Service &service)
    {
      QString id = service.id();
      if (id.isEmpty()) {
        return;
      }
      
      this->_hooks[id].clear();
    }

    void GameExecutorService::execute(const GGS::Core::Service &service)
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

      QMutexLocker lock(&this->_lock);

      if (this->_startedServices.contains(id)) {
        emit this->finished(service, AlreadyStartedError);
        return;
      } else {
        this->_startedServices << id;
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

      SIGNAL_CONNECT_CHECK(connect(loop, SIGNAL(canExecuteCompleted(const GGS::Core::Service &)),
        this, SIGNAL(canExecuteCompleted(const GGS::Core::Service &)), Qt::QueuedConnection));

      SIGNAL_CONNECT_CHECK(connect(loop, SIGNAL(preExecuteCompleted(const GGS::Core::Service &)),
        this, SIGNAL(preExecuteCompleted(const GGS::Core::Service &)), Qt::QueuedConnection));

      SIGNAL_CONNECT_CHECK(connect(loop, SIGNAL(started(const GGS::Core::Service &)),
        this, SIGNAL(started(const GGS::Core::Service &)), Qt::QueuedConnection));

      SIGNAL_CONNECT_CHECK(connect(loop, SIGNAL(finished(const GGS::Core::Service &, GGS::GameExecutor::FinishState)),
        this, SLOT(privateFinished(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), Qt::QueuedConnection));

      loop->execute();
    }

    void GameExecutorService::privateFinished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      QObject::sender()->deleteLater();

      QMutexLocker lock(&this->_lock);
      this->_startedServices.remove(service.id());
     
      emit this->finished(service, state);
    }

    bool GameExecutorService::isAnyGameStarted()
    {
      return this->_startedServices.count() > 0;
    }

  }
}