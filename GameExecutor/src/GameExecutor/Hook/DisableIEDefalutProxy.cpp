#include <GameExecutor/Hook/DisableIEDefalutProxy.h>

#include <Core/UI/Message>

#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include <Windows.h>
#include <Wininet.h>

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

        QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", QSettings::NativeFormat);
        if (0 == reg.value("ProxyEnable", "0").toInt() && 0 == reg.value("GlobalUserOffline", "0").toInt()) {
          emit this->preExecuteCompleted(true);
          return;
        }

        //UNDONE Локализация
        Message::StandardButton reply = Message::warning(
          QString::fromUtf8("Внимание!"), 
          QString::fromUtf8("Во избежание проблем с подключением к игровому серверу не рекомендуется использовать функции \"Автономная работа\" и \"Использовать прокси сервер для подключений LAN\" в браузере Internet Explorer. Вы хотите, чтобы GameNet автоматически отключил эти функции?"), 
          static_cast<Message::StandardButton>(Message::Yes | Message::No));
        
        if (reply == Message::No) {
           emit this->preExecuteCompleted(true);
           return;
        }

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