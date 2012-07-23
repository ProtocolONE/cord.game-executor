#include <GameExecutor/Hook/UxsmsTurnOffHook.h>

#include <QtCore/QDebug>

#include <Windows.h>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      UxsmsTurnOffHook::UxsmsTurnOffHook(QObject *parent) : HookInterface(parent)
      {
      }

      UxsmsTurnOffHook::~UxsmsTurnOffHook()
      {
      }

      void UxsmsTurnOffHook::CanExecute(Core::Service &service)
      {
        this->_shouldTurnOnUxsms = this->isServiceStarted();
        if (this->_shouldTurnOnUxsms)
          this->stopService();

        emit this->canExecuteCompleted(GGS::GameExecutor::Success);
      }

      void UxsmsTurnOffHook::PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state)
      {
        if (this->_shouldTurnOnUxsms)
          this->startService();

        emit this->postExecuteCompleted();
      }

      bool UxsmsTurnOffHook::isServiceStarted()
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

      void UxsmsTurnOffHook::stopService()
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

      void UxsmsTurnOffHook::startService()
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

    }
  }
}