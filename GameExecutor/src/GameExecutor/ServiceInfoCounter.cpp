#include <GameExecutor/ServiceInfoCounter.h>

#include <QtCore/QTime>
#include <QtCore/QDateTime>
#include <QtCore/QMutexLocker>

namespace GGS {
  namespace GameExecutor {
    ServiceInfoCounter::ServiceInfoCounter(QObject *parent) : QObject(parent)
    {
      this->_storage.beginGroup("gameExecutor");
      this->_storage.beginGroup("serviceInfo");
    }

    ServiceInfoCounter::~ServiceInfoCounter()
    {
    }

    void ServiceInfoCounter::started(const Core::Service &service)
    {
      QMutexLocker lock(&this->_lock);

      this->_storage.beginGroup(service.id());

      this->_storage.setValue("lastExecutionTime", QDateTime::currentDateTime());

      this->_storage.endGroup();
    }

    void ServiceInfoCounter::finished(const Core::Service &service, GGS::GameExecutor::FinishState state)
    {
      QMutexLocker lock(&this->_lock);

      this->_storage.beginGroup(service.id());

      QString key = (GGS::GameExecutor::Success == state) ? "successCount" : "failedCount";
      this->_storage.setValue(key, this->_storage.value(key, 0).toInt() + 1);

      QDateTime currentTime = QDateTime::currentDateTime();
      QDateTime startedTime = this->_storage.value("lastExecutionTime", currentTime).toDateTime();
      
      int playedTime = this->_storage.value("playedTime", 0).toInt();

      int diff = startedTime.secsTo(currentTime);
      if (diff < 0)
        diff = 0;

      this->_storage.setValue("playedTime", playedTime + diff);

      this->_storage.endGroup();
    }

    GGS::GameExecutor::ServiceInfo ServiceInfoCounter::queryInfo(const Core::Service &service)
    {
      Settings::Settings storage;
      storage.beginGroup("gameExecutor");
      storage.beginGroup("serviceInfo");
      storage.beginGroup(service.id());
      
      ServiceInfo info(
        storage.value("successCount", 0).toInt(),
        storage.value("failedCount", 0).toInt(),
        storage.value("playedTime", 0).toInt(),
        storage.value("lastExecutionTime", 0).toDateTime()
      );
      
      return info;
    }
  }
}
