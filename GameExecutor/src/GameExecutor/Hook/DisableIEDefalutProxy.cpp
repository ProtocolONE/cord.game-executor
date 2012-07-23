#include <GameExecutor/Hook/DisableIEDefalutProxy.h>

#include <Core/UI/Message>
#include <Core/System/Registry/RegistryKey.h>

#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include <Windows.h>
#include <Wininet.h>

using namespace GGS::Core::System::Registry;

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      using Core::UI::Message;

      DisableIEDefalutProxy::DisableIEDefalutProxy(QObject *parent) : HookInterface(parent)
      {
      }

      DisableIEDefalutProxy::~DisableIEDefalutProxy()
      {
      }

      void DisableIEDefalutProxy::PreExecute(Core::Service &service)
      {
        DEBUG_LOG << "for" << service.id();

        RegistryKey registry(RegistryKey::HKCU, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");

        DWORD proxyEnable = 0;
        DWORD globalUserOffline = 0;
        registry.value("ProxyEnable", proxyEnable);
        registry.value("GlobalUserOffline", globalUserOffline);

        if (proxyEnable == 0
          && globalUserOffline == 0) {
            emit this->preExecuteCompleted(GGS::GameExecutor::Success);
            return;
        }

        //UNDONE Локализация
        Message::StandardButton reply = Message::warning(
          QObject::tr("TITLE_ATTENTION"),
          QObject::tr("WARNING_DISABLE_IE_DEFALUT_PROXY"),
          static_cast<Message::StandardButton>(Message::Yes | Message::No));
        
        if (reply == Message::No) {
           emit this->preExecuteCompleted(GGS::GameExecutor::Success);
           return;
        } 

        registry.setValue("ProxyEnable", 0);
        registry.setValue("GlobalUserOffline", 0);

        // These lines implement the Interface in the beginning of program 
        // They cause the OS to refresh the settings, causing IP to realy update
        // http://msdn.microsoft.com/en-us/library/windows/desktop/aa385114(v=vs.85).aspx
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
        InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);

        emit this->preExecuteCompleted(GGS::GameExecutor::Success);
      }
    }
  }
}