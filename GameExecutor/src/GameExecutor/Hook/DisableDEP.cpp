#include <GameExecutor/Hook/DisableDEP.h>

#include <Core/UI/Message>

#include <QtCore/QSettings>
#include <QtCore/QDebug>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      DisableDEP::DisableDEP(QObject *parent) : HookInterface(parent)
      {
      }

      DisableDEP::~DisableDEP()
      {
      }

      void DisableDEP::PreExecute(Core::Service &service)
      {
        DEBUG_LOG << "for" << service.id();

        QUrl serviceUrl = service.url();

        QSettings reg("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control", QSettings::NativeFormat);
        QString systemOptions = reg.value("SystemStartOptions", "").toString();
        if (systemOptions.contains("NOEXECUTE=ALWAYSON")) {
          //UNDONE Локализация
          Core::UI::Message::warning(
            QString::fromUtf8("Внимание!"), 
            QString::fromUtf8("Игра не может быть запущена. В вашей ОС включена система предотвращения выполнения данных (DEP). Необходимо отключить DEP вручную. Если у вас возникнут проблемы с отключением, обратитесь в службу поддержки GameNet: https://support.gamenet.ru/"));

          emit this->preExecuteCompleted(false);
          return;
        }

        QSettings regLayer(
          "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 
          QSettings::NativeFormat);
        
        regLayer.setValue(serviceUrl.path().replace("/", "\\"), "DisableNXShowUI");
        emit this->preExecuteCompleted(true);
      }
    }
  }
}