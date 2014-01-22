/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QVariant>

#include <RestApi/GameNetCredential>
#include <RestApi/Commands/User/SetUserActivity>

#include <windows.h>

using GGS::RestApi::GameNetCredential;
using GGS::RestApi::Commands::User::SetUserActivity;

namespace GGS{
  namespace GameExecutor{
    namespace Executor {

      class QStringToWChar {
      public:
        QStringToWChar(const QString& str) : _data(NULL)
        {
          this->_size = str.size();
          this->_data = new wchar_t[this->_size + 1]();
          str.toWCharArray(this->_data);
        }

        ~QStringToWChar() {
          delete [] _data;
        }

        wchar_t* data() { return this->_data; }
        size_t size() { return this->_size; }

      private:
        wchar_t *_data;
        size_t _size;
      };

      ExecutableFileClient::ExecutableFileClient(QObject *parent) 
        : QObject(parent)
        , _needToRegistryRestore(false)
      {
        DEBUG_LOG;
        SIGNAL_CONNECT_CHECK(connect(&this->_executeFeatureWatcher, SIGNAL(finished()), this, SLOT(processFinished())));
      }

      ExecutableFileClient::~ExecutableFileClient()
      {
      }

      void ExecutableFileClient::sendMessage(QString message)
      {
        QStringList params = message.split("|", QString::KeepEmptyParts, Qt::CaseInsensitive);
        if (params.at(0) != "start") {
          CRITICAL_LOG << "unsupported command" << params.at(0);
          emit this->exit(Fail);
          return;
        }

        QString exeFile = params.at(1);

        if (!QFile::exists(exeFile)) {
          CRITICAL_LOG << "file not exists '" << exeFile << "'";
          emit this->exit(Fail);
          return;
        }

        bool gameIdCastResult = false;
        this->_gameId = params.at(6).toInt(&gameIdCastResult);

        if (false == gameIdCastResult) {
          CRITICAL_LOG << params.at(6);
          emit this->exit(Fail);
          return;
        }

        this->_userId = params.at(4);
        this->_appKey = params.at(5);

        QString dir = params.at(2);
        QString args = params.at(3);
       
        QString injectedDll;
        if (params.length() > 7)
          injectedDll = params.at(7);

        QString injectedDll2;
        if (params.length() > 8)
          injectedDll2 = params.at(8);

        QTimer::singleShot(1000, this, SLOT(setUserActivity()));

        DEBUG_LOG << "Start " << this->_userId << exeFile << params.at(2)<< injectedDll;

        // HACK переписать это на обычные потоки
        this->_executeFeature = QtConcurrent::run(
          this, 
          &ExecutableFileClient::startProcess, 
          exeFile, 
          params.at(2), 
          params.at(3), 
          injectedDll,
          injectedDll2);

        this->_executeFeatureWatcher.setFuture(this->_executeFeature);
      }

      unsigned int ExecutableFileClient::startProcess(
        QString pathToExe, 
        QString workDirectory, 
        QString args, 
        QString dllPath /*= QString()*/,
        QString dllPath2)
      {
        // HACK ППц нашел XP где обратные слешы не работают.!
        QString path = QDir::toNativeSeparators(pathToExe);
        QString commandLine = QString("\"%1\" %2").arg(path, args);

        QStringToWChar exe(path);
        QStringToWChar cmd(commandLine);
        QStringToWChar dir(workDirectory);
        
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);

        emit this->started();

        this->eraseRegistry();
        if (!CreateProcessW(exe.data(), cmd.data(), 0, 0, FALSE, CREATE_SUSPENDED, NULL, dir.data(), &si, &pi)) {
          this->restoreRegistry();
          DEBUG_LOG << "Create process failed" << GetLastError();
          emit this->exit(Fail);
          return 0xFFFFFFFF;
        }

        if (pi.hProcess == INVALID_HANDLE_VALUE) {
          this->restoreRegistry();
          DEBUG_LOG << "Create process invalid handle" << GetLastError();
          emit this->exit(Fail);
          return 0xFFFFFFFE;
        }

        HANDLE handle = pi.hProcess;
        if (!dllPath.isEmpty()) {
          DEBUG_LOG << "Start inject";
          HMODULE hModule = GetModuleHandleW(L"Kernel32");
          QStringToWChar dll(dllPath);

          size_t len = (dll.size() + 1) * sizeof(wchar_t);
          void* pLibRemote = ::VirtualAllocEx(handle, NULL, len, MEM_COMMIT, PAGE_READWRITE);
          DWORD iLen = 0;
          WriteProcessMemory(handle, pLibRemote, static_cast<void*>(dll.data()), len, &iLen);

          HANDLE waitHandle = CreateEvent(NULL, FALSE, FALSE, L"Local\\QGNA_OVERLAY_EVENT");

          HANDLE hThread = CreateRemoteThread(
            handle, 
            NULL, 
            0, 
            (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryW"), 
            pLibRemote, 
            0, 
            &iLen);

          WaitForSingleObject(hThread, INFINITE);
          CloseHandle(hThread);

          WaitForSingleObject(waitHandle, 5000);
          CloseHandle(waitHandle);
        }

        // UNDONE рефакторнуть это как-то. Например грузить все дополнительные дллки через общий хелпер.
        if (!dllPath2.isEmpty()) {
          this->_shareArgs(pi.dwProcessId);
          DEBUG_LOG << "Start inject2";
          HMODULE hModule = GetModuleHandleW(L"Kernel32");
          QStringToWChar dll(dllPath2);

          size_t len = (dll.size() + 1) * sizeof(wchar_t);
          void* pLibRemote = ::VirtualAllocEx(handle, NULL, len, MEM_COMMIT, PAGE_READWRITE);
          DWORD iLen = 0;
          WriteProcessMemory(handle, pLibRemote, static_cast<void*>(dll.data()), len, &iLen);

          HANDLE waitHandle = CreateEvent(NULL, FALSE, FALSE, L"Local\\QGNA_OVERLAY_EVENT2");

          HANDLE hThread = CreateRemoteThread(
            handle, 
            NULL, 
            0, 
            (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryW"), 
            pLibRemote, 
            0, 
            &iLen);

          WaitForSingleObject(hThread, INFINITE);
          CloseHandle(hThread);

          WaitForSingleObject(waitHandle, 15000);
          CloseHandle(waitHandle);
          this->_deleteSharedArgs();
        }

        ResumeThread(pi.hThread);

        // INFO этот код важен. Необходимо подождать пока проинициализируется user32.dll и только потом восстанавливать
        // реестр.
        Sleep(100);
        this->restoreRegistry();
        
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        return exitCode;
      }

      void ExecutableFileClient::processFinished()
      {
        unsigned int result = this->_executeFeatureWatcher.result();
        int realExitCode = (result != 0xC0000005 && result != 0) ? Fail : Success;
        DEBUG_LOG << "with exit code" << result << "real result" << realExitCode;

        this->setUserActivityLogout(realExitCode);
        emit this->finished(realExitCode);
      }

      void ExecutableFileClient::setUserActivity()
      {
        SetUserActivity *cmd = new SetUserActivity();

        cmd->setGameId(this->_gameId);  
        cmd->setLogout(0);

        cmd->setAuthRequire(false);
        cmd->appendParameter("userId", this->_userId);
        cmd->appendParameter("appKey", this->_appKey);

        SIGNAL_CONNECT_CHECK(connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(setUserActivityResult(GGS::RestApi::CommandBase::CommandResults))));

        cmd->execute();
      }

      void ExecutableFileClient::setUserActivityResult(GGS::RestApi::CommandBase::CommandResults result)
      {
        SetUserActivity *cmd = qobject_cast<SetUserActivity *>(QObject::sender());
        if (!cmd) {
          WARNING_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        cmd->deleteLater();

        if (result != CommandBase::NoError) {
          WARNING_LOG << "error" << result << cmd->getGenericErrorMessageCode();
          QTimer::singleShot(300000, this, SLOT(setUserActivity())); // default timeOut is 5 minutes;
          return;
        }

        int timeOut = cmd->getTimeout();
        if (timeOut <= 0) {
          WARNING_LOG <<  "wrong timeOut" << timeOut;
          QTimer::singleShot(300000, this, SLOT(setUserActivity()));
          return;  
        }

        QTimer::singleShot(timeOut * 1000, this, SLOT(setUserActivity()));
      }

      void ExecutableFileClient::setUserActivityLogout(int code)
      {
        this->_code = code;

        SetUserActivity *cmd = new SetUserActivity();

        cmd->setGameId(this->_gameId);
        cmd->setLogout(1);
        cmd->setAuthRequire(false);
        cmd->appendParameter("userId", this->_userId);
        cmd->appendParameter("appKey", this->_appKey);

        SIGNAL_CONNECT_CHECK(connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(setUserActivityLogoutResult(GGS::RestApi::CommandBase::CommandResults))));

        cmd->execute();
      }

      void ExecutableFileClient::setUserActivityLogoutResult(GGS::RestApi::CommandBase::CommandResults result)
      {
        SetUserActivity *cmd = qobject_cast<SetUserActivity *>(QObject::sender());
        if (!cmd) {
          WARNING_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        cmd->deleteLater();

        emit this->exit(this->_code);
      }

      void ExecutableFileClient::setShareArgs(std::function<void (unsigned int)> value)
      {
        this->_shareArgs = value;
      }

      void ExecutableFileClient::setDeleteSharedArgs(std::function<void ()> value)
      {
        this->_deleteSharedArgs = value;
      }

      void ExecutableFileClient::eraseRegistry()
      {
        this->_needToRegistryRestore = false;

        QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
          QSettings::NativeFormat);

        if (!settings.contains("LoadAppInit_DLLs"))
          return;
        
        QVariant value = settings.value("LoadAppInit_DLLs");
        if (value.isValid()) {
          this->_needToRegistryRestore = true;
          this->_registryValue = value.toInt();
          settings.setValue("LoadAppInit_DLLs", 0);
        }
      }

      void ExecutableFileClient::restoreRegistry()
      { 
        if (this->_needToRegistryRestore) {
          QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
            QSettings::NativeFormat);
          settings.setValue("LoadAppInit_DLLs", this->_registryValue);
        }
      }

    }
  }
}
