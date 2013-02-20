#include <GameExecutor/ServiceInfo.h>

namespace GGS {
  namespace GameExecutor {
    ServiceInfo::ServiceInfo(int successCount, int failedCount, int playedTime, const QDateTime &lastExecutionTime)
      : _successCount(successCount), 
        _failedCount(failedCount), 
        _playedTime(playedTime), 
        _lastExecutionTime(lastExecutionTime)
    {
    }

    ServiceInfo::~ServiceInfo()
    {
    }

    int ServiceInfo::successCount() const
    {
      return this->_successCount;
    }

    int ServiceInfo::failedCount() const
    {
      return this->_failedCount;
    }

    int ServiceInfo::playedTime() const
    {
      return this->_playedTime;
    }

    const QDateTime& ServiceInfo::lastExecutionTime() const
    {
      return this->_lastExecutionTime;
    }
  }
}