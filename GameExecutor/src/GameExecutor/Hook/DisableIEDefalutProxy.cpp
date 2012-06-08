#include <GameExecutor/Hook/DisableIEDefalutProxy.h>

#include <Windows.h>
#include <Wininet.h>

#include <QtCore/QSettings>
#include <QtCore/QDebug>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      DisableIEDefalutProxy::DisableIEDefalutProxy(QObject *parent) : HookInterface(parent)
      {
      }

      DisableIEDefalutProxy::~DisableIEDefalutProxy()
      {
      }

      void DisableIEDefalutProxy::PreExecute(Core::Service &service)
      {
        DEBUG_LOG << "for" << service.id();

        QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", QSettings::NativeFormat);
        if (0 == reg.value("ProxyEnable", "0").toInt() && 0 == reg.value("GlobalUserOffline", "0").toInt()) {
          emit this->preExecuteCompleted(true);
          return;
        }

        //UNDONE ѕока у нас нет возможности это спросить
        /*
        if (somePossibility->userDon`tWantToDisableThis) {
        return false;
        }
        */

        reg.setValue("ProxyEnable", 0);
        reg.setValue("GlobalUserOffline", 0);

        // These lines implement the Interface in the beginning of program 
        // They cause the OS to refresh the settings, causing IP to realy update
        // http://msdn.microsoft.com/en-us/library/windows/desktop/aa385114(v=vs.85).aspx
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
        InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);

        emit this->preExecuteCompleted(true);
      }
    }
  }
}