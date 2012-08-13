/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/Hook/DisableAeroHook.h>
#include <Core/System/Registry/RegistryKey.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <Windows.h>

using namespace GGS::Core::System::Registry;

namespace GGS {
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
        QString pathToExe = QString("%1/%2/Engine.exe").arg(service.installPath(), service.areaString());
        pathToExe = QDir::toNativeSeparators(pathToExe);

        RegistryKey registry(RegistryKey::HKCU, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");
        if (!registry.setValue(pathToExe, "DISABLETHEMES DISABLEDWM")) {
          DEBUG_LOG << "DisableAeroHook failed";
        }

        emit this->canExecuteCompleted(service, GGS::GameExecutor::Success);
      }
    }
  }
}