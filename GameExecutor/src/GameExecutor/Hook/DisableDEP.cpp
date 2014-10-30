#include <GameExecutor/Hook/DisableDEP.h>

#include <Core/UI/Message>
#include <Core/System/Registry/RegistryKey.h>

#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include <QtCore/QDir>

using namespace GGS::Core::System::Registry;

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

        RegistryKey registry(RegistryKey::HKLM, "SYSTEM\\CurrentControlSet\\Control");
        QString systemOptions;
        registry.value("SystemStartOptions", systemOptions);

        if (systemOptions.contains("NOEXECUTE=ALWAYSON")) {
          Core::UI::Message::warning(QObject::tr("TITLE_ATTENTION"), QObject::tr("WARNING_DISABLEDEP"));

          emit this->preExecuteCompleted(service, GGS::GameExecutor::PreExecutionHookBreak);
          return;
        }

        RegistryKey registry2(RegistryKey::HKLM, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");
        QString path = QDir::toNativeSeparators(serviceUrl.path());
        registry2.setValue(path, "DisableNXShowUI");
        emit this->preExecuteCompleted(service, GGS::GameExecutor::Success);
      }

      QString DisableDEP::id()
      {
        return "54B0860B-215C-462F-A80E-F7664DEA984F";
      }

    }
  }
}