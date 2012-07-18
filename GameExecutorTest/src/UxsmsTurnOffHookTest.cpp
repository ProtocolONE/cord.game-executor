#include <GameExecutor/Hook/UxsmsTurnOffHook.h>

#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtCore/QEventLoop>
#include <QtTest/QSignalSpy>
#include <gtest/gtest.h>
#include <Windows.h>

#define ServerStartTimeout 1000

class UxsmsTurnOffHookTest : public ::testing::Test {
public:
  void SetUp() {
    service.setArea(GGS::Core::Service::Live);
    service.setId("300009010000000000");
  }

  void executeCanHook()
  {
    QSignalSpy spy(&hook, SIGNAL(canExecuteCompleted(GGS::GameExecutor::FinishState)));
    QEventLoop loop;
    QObject::connect(&hook, SIGNAL(canExecuteCompleted(GGS::GameExecutor::FinishState)), &loop, SLOT(quit()), Qt::QueuedConnection);

    hook.CanExecute(service);
    loop.exec();
    ASSERT_EQ(1, spy.count());
    resultEvent = spy.takeFirst();
  }

  void executePostHook()
  {
    QEventLoop loop;
    QObject::connect(&hook, SIGNAL(postExecuteCompleted()), &loop, SLOT(quit()), Qt::QueuedConnection);
    hook.PostExecute(service, GGS::GameExecutor::FinishState::Success);
    loop.exec();
  }

  GGS::Core::Service service;
  GGS::GameExecutor::Hook::UxsmsTurnOffHook hook;
  QList<QVariant> resultEvent;

  bool isServiceStarted()
  {
    SC_HANDLE scmanager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmanager == NULL) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "OpenSCManager error " << errorCode;
      return false;
    }

    SC_HANDLE uxsms = OpenServiceW(scmanager, L"uxsms", SERVICE_ALL_ACCESS);
    if (uxsms == NULL) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "OpenServiceW error " << errorCode;
      CloseServiceHandle(scmanager);
      return false;
    }

    SERVICE_STATUS serviceStatus;
    bool result = QueryServiceStatus(uxsms, &serviceStatus);

    CloseServiceHandle(uxsms);
    CloseServiceHandle(scmanager);
    return result && 
      (serviceStatus.dwCurrentState == SERVICE_RUNNING
      || serviceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING
      || serviceStatus.dwCurrentState == SERVICE_START_PENDING);
  }

  void stopService()
  {
    SC_HANDLE scmanager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmanager == NULL) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "OpenSCManager error " << errorCode;
      return;
    }

    SC_HANDLE uxsms = OpenServiceW(scmanager, L"uxsms", SERVICE_ALL_ACCESS);
    if (uxsms == NULL) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "OpenServiceW error " << errorCode;
      CloseServiceHandle(scmanager);
      return;
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(uxsms, SERVICE_CONTROL_STOP, &serviceStatus)) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "ControlService SERVICE_CONTROL_STOP error " << errorCode;
    }

    CloseServiceHandle(uxsms);
    CloseServiceHandle(scmanager);
  }

  void startService()
  {
    SC_HANDLE scmanager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmanager == NULL) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "OpenSCManager error " << errorCode;
      return;
    }

    SC_HANDLE uxsms = OpenServiceW(scmanager, L"uxsms", SERVICE_ALL_ACCESS);
    if (uxsms == NULL) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "OpenServiceW error " << errorCode;
      CloseServiceHandle(scmanager);
      return;
    }

    if (!StartService(uxsms, NULL, NULL)) {
      DWORD errorCode = GetLastError();
      DEBUG_LOG << "StartService error " << errorCode;
    }

    CloseServiceHandle(uxsms);
    CloseServiceHandle(scmanager);
  }
};

TEST_F(UxsmsTurnOffHookTest, DISABLED_TurnOffAndTurnOnTest)
{
  if (!this->isServiceStarted())
    this->startService();

  Sleep(ServerStartTimeout);
  ASSERT_TRUE(this->isServiceStarted());
  executeCanHook();
  Sleep(ServerStartTimeout);
  ASSERT_FALSE(this->isServiceStarted());
  executePostHook();
  Sleep(ServerStartTimeout);
  ASSERT_TRUE(this->isServiceStarted());
}

TEST_F(UxsmsTurnOffHookTest, DISABLED_NothingShouldBeDoneTest)
{
  if (this->isServiceStarted())
    this->stopService();
  Sleep(ServerStartTimeout);

  ASSERT_FALSE(this->isServiceStarted());
  executeCanHook();
  Sleep(ServerStartTimeout);
  ASSERT_FALSE(this->isServiceStarted());

  Sleep(ServerStartTimeout);
  executePostHook();
  ASSERT_FALSE(this->isServiceStarted());

  this->startService();
}