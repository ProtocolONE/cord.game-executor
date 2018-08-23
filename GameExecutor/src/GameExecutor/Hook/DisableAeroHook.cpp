#include <GameExecutor/Hook/DisableAeroHook.h>
#include <Core/System/Registry/RegistryKey.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QSysInfo>

#include <Windows.h>

using namespace P1::Core::System::Registry;

namespace P1 {
  namespace GameExecutor {
    namespace Hook {

      DisableAeroHook::DisableAeroHook(QObject *parent) : HookInterface(parent)
      {
      }

      DisableAeroHook::~DisableAeroHook()
      {
      }

      void DisableAeroHook::CanExecute(Core::Service &service)
      {
        QSysInfo::WinVersion version = QSysInfo::windowsVersion();
        bool oldVersion = version == QSysInfo::WV_NT 
          || version == QSysInfo::WV_2000
          || version == QSysInfo::WV_XP
          || version == QSysInfo::WV_2003;

        if (!oldVersion)
          this->disableAero(service);

        emit this->canExecuteCompleted(service, P1::GameExecutor::Success);
      }

      QString DisableAeroHook::id()
      {
        return "B0215EBC-27F0-4C3C-BC21-0C3611AFEEF4";
      }

      void DisableAeroHook::disableAero(const Core::Service &service)
      {
        QString pathToExe = QString("%1/%2/Engine.exe").arg(service.installPath(), service.areaString());
        pathToExe = QDir::toNativeSeparators(pathToExe);

        RegistryKey registry(RegistryKey::HKCU, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");
        if (!registry.setValue(pathToExe, "DISABLETHEMES DISABLEDWM")) {
          DEBUG_LOG << "DisableAeroHook failed";
        }
      }

    }
  }
}