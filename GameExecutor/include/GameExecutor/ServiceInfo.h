#pragma once

#include <GameExecutor/gameexecutor_global.h>

#include <QtCore/QTime>
#include <QtCore/QDateTime>

namespace P1 {
  namespace GameExecutor {
    class GAMEEXECUTOR_EXPORT ServiceInfo
    {
    public:
      ServiceInfo(int successCount, int failedCount, int playedTime, const QDateTime &lastExecutionTime);
      ~ServiceInfo();

      int successCount() const;
      int failedCount() const;
      int playedTime() const;
      const QDateTime &lastExecutionTime() const;
    
    private:
      int _successCount;
      int _failedCount;
      int _playedTime;
      QDateTime _lastExecutionTime;
    };
  }
}
