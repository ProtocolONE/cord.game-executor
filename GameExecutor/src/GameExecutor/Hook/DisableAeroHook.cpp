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

#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <Windows.h>

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
        QString key("Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");
        this->writeRegValue(key, pathToExe, "DISABLETHEMES DISABLEDWM");
        emit this->canExecuteCompleted(GGS::GameExecutor::Success);
      }

      void DisableAeroHook::writeRegValue(const QString& key, const QString& paramName, const QString& value)
      {
        wchar_t keyArray[MAX_PATH] = {0};
        key.toWCharArray(keyArray);
        HKEY hkey;
        DWORD res = RegCreateKeyEx(HKEY_CURRENT_USER, keyArray, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY | KEY_READ | KEY_WRITE, NULL, &hkey, NULL);
        if (res != ERROR_SUCCESS) {
          DEBUG_LOG << "Can't open " << key << "Error:" << res;
          return;
        }

        wchar_t paramNameArray[MAX_PATH] = {0};
        paramName.toWCharArray(paramNameArray);

        wchar_t valueArray[MAX_PATH + 1] = {0};
        value.toWCharArray(valueArray);
        RegSetValueEx(hkey, reinterpret_cast<LPCWSTR>(paramNameArray), 0, REG_SZ, reinterpret_cast<BYTE*>(valueArray), value.size()*2 + 2);
        RegCloseKey(hkey);
      }

    }
  }
}