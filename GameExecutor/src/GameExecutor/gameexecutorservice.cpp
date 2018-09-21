#include <GameExecutor/ExecutionLoop_p.h>
#include <GameExecutor/gameexecutorservice.h>

#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>

namespace P1 {
  namespace GameExecutor {

    GameExecutorService::GameExecutorService(QObject *parent) 
      : QObject(parent)
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

    bool GameExecutorService::addHook(const P1::Core::Service &service, HookInterface *hook, int priority /*= 0*/)
    {
      QString id = service.id();
      if (id.isEmpty())
        return false;

      this->_hooks[id].insert(priority, hook);
      return true;
    }

    void GameExecutorService::clearHooks(const P1::Core::Service &service)
    {
      QString id = service.id();
      if (id.isEmpty())
        return;
      
      this->_hooks[id].clear();
    }

    void GameExecutorService::execute(
      const P1::Core::Service &service, 
      const P1::RestApi::ProtocolOneCredential &credential)
    {
      QString id = service.id();
      if (id.isEmpty()) {
        DEBUG_LOG << "Can't start game. Id is empty";
        emit this->finished(service, InvalidService);
        return;
      }

      QUrl url = service.url();
      QString scheme = url.scheme();

      QMutexLocker lock(&this->_lock);

      if (!this->hasExecutor(scheme)) {
        DEBUG_LOG << "Unknown executor scheme" << scheme;
        emit this->finished(service, UnknownSchemeError);
        return;
      }

      if (this->_startedServices.contains(id)) {
        emit this->finished(service, AlreadyStartedError);
        return;
      }

      this->_startedServices << id;

      QList<HookInterface *> list;
      Q_FOREACH (HookInterface *hook, this->_hooks[id]) {
        list.append(hook);
      }

      ExecutionLoopPrivate *loop = new ExecutionLoopPrivate(this);
      loop->setService(service);
      loop->setHookList(list);
      loop->setExecutor(this->_executors[scheme]);
      loop->setExecutorService(this);
      loop->setCredential(credential);

      QObject::connect(this, &GameExecutorService::stopExecution, 
        loop, &ExecutionLoopPrivate::onStopExecution);

      QObject::connect(loop, &ExecutionLoopPrivate::canExecuteCompleted,
        this, &GameExecutorService::canExecuteCompleted, Qt::QueuedConnection);

      QObject::connect(loop, &ExecutionLoopPrivate::preExecuteCompleted, 
        this, &GameExecutorService::preExecuteCompleted, Qt::QueuedConnection);

      QObject::connect(loop, &ExecutionLoopPrivate::started,
        this, &GameExecutorService::started, Qt::QueuedConnection);

      QObject::connect(loop, &ExecutionLoopPrivate::finished,
        this, &GameExecutorService::privateFinished, Qt::QueuedConnection);

      loop->execute();
    }

    void GameExecutorService::terminate(const QString& serviceId)
    {
      emit this->stopExecution(serviceId);
    }

    void GameExecutorService::terminateAll()
    {
      this->terminate(QString());
    }

    void GameExecutorService::privateFinished(const P1::Core::Service &service, P1::GameExecutor::FinishState state)
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

    void GameExecutorService::shutdown()
    {
      QMutexLocker lock(&this->_lock);

      Q_FOREACH(ExecutorBase* executor, this->_executors)
        delete executor;

      this->_executors.clear();
    }

    bool GameExecutorService::isGameStarted(const QString& serviceId)
    {
      return this->_startedServices.contains(serviceId);
    }

  }
}