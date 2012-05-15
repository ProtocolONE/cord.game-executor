/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/ServiceInfoCounter.h>
#include <GameExecutor/ServiceInfo.h>

#include <Core/Service>
#include <Settings/Settings>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

#include <gtest/gtest.h>
#include <Windows.h>

using namespace GGS;

class ServiceInfoCounterTest : public ::testing::Test 
{
protected:
  static void SetUpTestCase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbSettingsPath = QString("%1/settings.sql").arg(QCoreApplication::applicationDirPath());
    if (QFile::exists(dbSettingsPath)) 
      QFile::remove(dbSettingsPath);

    db.setDatabaseName(dbSettingsPath);

    if (db.open("admin", "admin")) {
      if (!db.tables().contains("app_settings")) {
        QSqlQuery query = db.exec("CREATE TABLE app_settings "
          "( "
          " key_column text NOT NULL, "
          " value_column text, "
          " CONSTRAINT app_settings_pk PRIMARY KEY (key_column) "
          ")");
        
        if (query.lastError().isValid())
          qFatal(query.lastError().text().toStdString().c_str());
      } 
    }
 
    GGS::Settings::Settings::setConnection(db.connectionName());
  }

  GameExecutor::ServiceInfoCounter counter;
};

TEST_F(ServiceInfoCounterTest, BasicTest)
{
  Core::Service srv;
  srv.setId("testId");

  GameExecutor::ServiceInfo info = GameExecutor::ServiceInfoCounter::queryInfo(srv);
  ASSERT_EQ(0, info.successCount());
  ASSERT_EQ(0, info.failedCount());
  //К сожалению у QDateTime/QTime нет правостороннего `=`
  ASSERT_EQ(-1, info.lastExecutionTime().toTime_t());
  ASSERT_EQ(0, info.playedTime().second());

  GameExecutor::ServiceInfoCounter counter;
  counter.started(srv);

  Sleep(1000);

  counter.finished(srv, GGS::GameExecutor::Success);
  counter.finished(srv, GGS::GameExecutor::ExternalFatalError);

  info = GameExecutor::ServiceInfoCounter::queryInfo(srv);
  ASSERT_EQ(1, info.successCount());
  ASSERT_EQ(1, info.failedCount());
  // Машина с тестами выполняет 1 секунду за 8 
  ASSERT_GE(info.lastExecutionTime().toTime_t(), QDateTime::currentDateTime().toTime_t() - 15);
  ASSERT_GE(info.playedTime().second(), 1);
}