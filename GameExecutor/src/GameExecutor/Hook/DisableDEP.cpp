#include <GameExecutor/Hook/DisableDEP.h>

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
          //UNDONE ��� ��� ���� ����������� 
          /*
          someService.ShowVeryImpormantWarningMessageAboutDep(ErrorMessa);
          */
        }

        QSettings regLayer(
          "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 
          QSettings::NativeFormat);
          
        regLayer.setValue(serviceUrl.path(), "DisableNXShowUI");

        emit this->preExecuteCompleted(true);
      }
    }
  }
}