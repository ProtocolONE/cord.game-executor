#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/Enum.h>
#include <GameExecutor/ServiceInfo.h>

#include <Settings/Settings.h>

#include <Core/Service.h>

#include <QtCore/QMutex>

namespace P1 {
  namespace GameExecutor {
    class GAMEEXECUTOR_EXPORT ServiceInfoCounter : public QObject
    {
      Q_OBJECT
    public:
      ServiceInfoCounter(QObject *parent = 0);
      ~ServiceInfoCounter();

      static ServiceInfo queryInfo(const Core::Service &service);

    public slots:
      void started(const P1::Core::Service &service);
      void finished(const P1::Core::Service &service, P1::GameExecutor::FinishState state);

    private:
      P1::Settings::Settings _storage;
      QMutex _lock;
    };
  }
}
