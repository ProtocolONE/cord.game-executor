#include <GameExecutor/Hook/DisableDEP.h>

#include <QtCore/QSettings>
#include <QtCore/QDebug>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      DisableDEP::DisableDEP(QObject *parent) : QObject(parent)
      {
      }

      DisableDEP::~DisableDEP()
      {
      }

      bool DisableDEP::CanExecute(const Core::Service &service)
      {
        return true;
      }

      void DisableDEP::PostExecute(const Core::Service &service, GGS::GameExecutor::FinishState state)
      {
      }

      bool DisableDEP::PreExecute(const Core::Service &service)
      {
        qDebug() << __FUNCTION__;

        QUrl serviceUrl = service.url();

        QSettings reg("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control", QSettings::NativeFormat);
        QString systemOptions = reg.value("SystemStartOptions", "").toString();
        if (systemOptions.contains("NOEXECUTE=ALWAYSON")) {
          //UNDONE Еще нет этой возможности 
          /*
          someService.ShowVeryImpormantWarningMessageAboutDep(ErrorMessa);
          */
        }

        QSettings regLayer(
          "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 
          QSettings::NativeFormat);
          
        regLayer.setValue(serviceUrl.path(), "DisableNXShowUI");

        return true;
      }
    }
  }
}