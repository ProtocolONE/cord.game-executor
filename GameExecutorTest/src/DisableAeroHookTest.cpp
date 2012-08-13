#include <GameExecutor/Hook/DisableAeroHook.h>

#include <QtCore/QList>
#include <QtCore/QCoreApplication>
#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtCore/QEventLoop>
#include <QtTest/QSignalSpy>
#include <gtest/gtest.h>
#include <Windows.h>

class DisableAeroHookTest : public ::testing::Test {
public:
  void SetUp() {
    service.setArea(GGS::Core::Service::Live);
    service.setId("300009010000000000");
    service.setInstallPath(QCoreApplication::applicationDirPath());
  }

  void executeCanHook()
  {
    QSignalSpy spy(&hook, SIGNAL(canExecuteCompleted(const GGS::Core::Service &, GGS::GameExecutor::FinishState)));
    QEventLoop loop;
    QObject::connect(&hook, SIGNAL(canExecuteCompleted(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), &loop, SLOT(quit()), Qt::QueuedConnection);

    hook.CanExecute(service);
    loop.exec();
    ASSERT_EQ(1, spy.count());
    resultEvent = spy.takeFirst();
  }

  void executePostHook()
  {
    QEventLoop loop;
    QObject::connect(&hook, SIGNAL(postExecuteCompleted(const GGS::Core::Service &)), &loop, SLOT(quit()), Qt::QueuedConnection);
    hook.PostExecute(service, GGS::GameExecutor::FinishState::Success);
    loop.exec();
  }

  GGS::Core::Service service;
  GGS::GameExecutor::Hook::DisableAeroHook hook;
  QList<QVariant> resultEvent;

};

TEST_F(DisableAeroHookTest, simpleTest)
{
  executeCanHook();
}