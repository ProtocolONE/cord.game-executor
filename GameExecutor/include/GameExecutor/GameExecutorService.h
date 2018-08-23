#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/HookInterface.h>

#include <Core/Service.h>
#include <RestApi/RestApiManager.h>

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QMultiMap>

#include <functional>

namespace P1 {
  namespace GameExecutor {

    /*!
      \class GameExecutorService
      \brief Управляет запуском игр.
      \ingroup GameExecutor

      Важно, что последовательность 

      \sa Core::Service, ExecutorBase, HookInterface
    */
    class GAMEEXECUTOR_EXPORT GameExecutorService : public QObject
    {
      Q_OBJECT
    public:
      explicit GameExecutorService(QObject *parent = 0);
      ~GameExecutorService();

      bool registerExecutor(ExecutorBase *executor);
      bool hasExecutor(const QString &scheme) const;

      virtual bool addHook(const P1::Core::Service &service, HookInterface* hook, int priority = 0);
      void clearHooks(const P1::Core::Service &service);

      void execute(
        const P1::Core::Service &service, 
        const P1::RestApi::GameNetCredential &credential);

      void terminateAll();
      void terminate(const QString& serviceId);

      bool isAnyGameStarted();
      bool isGameStarted(const QString& serviceId);

      void shutdown();

    signals:
      void canExecuteCompleted(const P1::Core::Service &service);
      void preExecuteCompleted(const P1::Core::Service &service);
      void started(const P1::Core::Service &service);
      void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);
      void stopExecution(const QString& serviceId);
      void dataCorrupted();

    private slots:
      void privateFinished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);
   
    private:
      QHash<QString, ExecutorBase*> _executors;
      QHash<QString, QMultiMap<int, HookInterface*>> _hooks;
      QSet<QString> _startedServices;
      QMutex _lock;

    };
  }
}
