#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/Enum.h>

#include <Core/Service.h>

#include <RestApi/RestApiManager.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameExecutor {
    class GameExecutorService;

    class GAMEEXECUTOR_EXPORT ExecutorBase : public QObject
    {
      Q_OBJECT
    public:
      explicit ExecutorBase(QObject *parent = 0);
      ExecutorBase(const QString &scheme, QObject *parent = 0);
      virtual ~ExecutorBase();

      virtual void execute(
        const Core::Service &service, 
        GameExecutorService *executorService, 
        const P1::RestApi::ProtocolOneCredential& credential) = 0;

      const QString &scheme() const;

      static FinishState finishStateFromRestApiErrorCode(int errorCode);

    signals:
      void started(const P1::Core::Service &service);
      void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);
        
    protected:
      QString _scheme;
    };
  }
}

