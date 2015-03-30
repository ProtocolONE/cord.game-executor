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
#include <GameExecutor/Executor/AppInitPatch.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QWinEventNotifier>
#include <QtCore/QCoreApplication>

#include <RestApi/GameNetCredential>
#include <RestApi/Commands/User/SetUserActivity>

#include <Common/ConfigReader.h>

using GGS::RestApi::GameNetCredential;
using GGS::RestApi::Commands::User::SetUserActivity;
using GGS::RestApi::CommandBase;

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

        ~QStringToWChar() 
        {
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
        , _appinitPatch(new AppInitPatch(this))
        , _processHandle(NULL)
        , _threadHandle(NULL)
        , _shareArgs(nullptr)
      {
      }

      ExecutableFileClient::~ExecutableFileClient()
      {
        this->stopProcess();
      }

      void ExecutableFileClient::startProcess(
        const QString& pathToExe,
        const QString& args,
        const QString& workDirectory,
        const QString& injectedDll)
      {
        DEBUG_LOG << "Start " << pathToExe << workDirectory;
        
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

        QString nvidia = this->_appinitPatch->nvidiaDriverPath();

        this->_appinitPatch->eraseRegistry();

        if (!CreateProcessW(exe.data(), cmd.data(), 0, 0, FALSE, CREATE_SUSPENDED, NULL, dir.data(), &si, &pi)) {
          this->_appinitPatch->restoreRegistry();
          DEBUG_LOG << "Create process failed" << GetLastError();
          emit this->finished(Fail);
          return;
        }

        if (pi.hProcess == INVALID_HANDLE_VALUE) {
          this->_appinitPatch->restoreRegistry();
          DEBUG_LOG << "Create process invalid handle" << GetLastError();
          emit this->finished(Fail);
          return;
        }

        QWinEventNotifier *monitor = new QWinEventNotifier(pi.hProcess, this);
        QObject::connect(monitor, &QWinEventNotifier::activated, 
          this, &ExecutableFileClient::handleActivated);

        this->_processHandle = pi.hProcess;
        this->_threadHandle = pi.hThread;

        this->_appinitPatch->patchAppinit(this->_processHandle);

        std::vector<std::wstring> toLoad;
        toLoad.push_back(L"user32.dll");

        if (!nvidia.isEmpty())
          toLoad.push_back(nvidia.toStdWString());

        bool cmdFix = false;
        bool sphFix = false;

        if (this->_paramHolder.count(CommandLineCheck))
          cmdFix = this->_paramHolder[CommandLineCheck];

        if (this->_paramHolder.count(SpeedHackCheck))
          sphFix = this->_paramHolder[SpeedHackCheck];
        
        GGS::GameExecutorHelper::ConfigReader cfg;

        cfg.setPid(pi.dwProcessId);
        cfg.setCommandLineFlag(cmdFix);
        cfg.setSpeedHackFlag(sphFix);
        cfg.setStrings(toLoad);
        cfg.setServiceId(this->_serviceId.toStdWString());
        cfg.write();

        if (!injectedDll.isEmpty())
          this->injectDll(this->_processHandle, injectedDll, QString("Local\\QGNA_OVERLAY_EVENT"));

#ifdef _DEBUG
        QString executorHelper = QCoreApplication::applicationDirPath() + "/GameExecutorHelperX86d.dll"; 
#else
        QString executorHelper = QCoreApplication::applicationDirPath() + "/GameExecutorHelperX86.dll";
#endif
        if (cmdFix)
          this->_shareArgs(pi.dwProcessId, this->_processHandle);

        this->injectDll(this->_processHandle, executorHelper, QString("Local\\QGNA_OVERLAY_EVENT2"));

        if (cmdFix)
          this->_deleteSharedArgs(); 

        ResumeThread(pi.hThread);
        this->_appinitPatch->restoreRegistry();
      }

      void ExecutableFileClient::stopProcess()
      {
        if (this->_processHandle != NULL)
          TerminateProcess(this->_processHandle, 0);
      }

      void ExecutableFileClient::setShareArgs(std::function<void (unsigned int, HANDLE)> value)
      {
        this->_shareArgs = value;
      }

      void ExecutableFileClient::setDeleteSharedArgs(std::function<void ()> value)
      {
        this->_deleteSharedArgs = value;
      }

      void ExecutableFileClient::injectDll(HANDLE handle, const QString& path, const QString& waitEvent /*= QString()*/)
      {
        HMODULE hModule = GetModuleHandleW(L"Kernel32");
        QStringToWChar dll(path);
        QStringToWChar waitEventName(waitEvent);
        HANDLE waitHandle = NULL;

        size_t len = (dll.size() + 1) * sizeof(wchar_t);
        void* pLibRemote = ::VirtualAllocEx(handle, NULL, len, MEM_COMMIT, PAGE_READWRITE);
        DWORD iLen = 0;
        WriteProcessMemory(handle, pLibRemote, static_cast<void*>(dll.data()), len, &iLen);

        if (!waitEvent.isEmpty())
          waitHandle = CreateEvent(NULL, FALSE, FALSE, waitEventName.data());

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

        if (waitHandle != NULL) {
          WaitForSingleObject(waitHandle, 15000);
          CloseHandle(waitHandle);
        }
      }

      void ExecutableFileClient::handleActivated(HANDLE handle)
      {
        QObject::sender()->deleteLater();
        DWORD exitCode = 0;
        GetExitCodeProcess(handle, &exitCode);

        this->closeHandles();
        
        int realExitCode = (exitCode != 0xC0000005 && exitCode != 0) ? Fail : Success;
        DEBUG_LOG << "with exit code" << exitCode << "real result" << realExitCode;

        emit this->finished(realExitCode);
      }

      void ExecutableFileClient::closeHandles()
      {
        if (this->_threadHandle)
          CloseHandle(this->_threadHandle);

        this->_threadHandle = 0;

        if (this->_processHandle)
          CloseHandle(this->_processHandle);

        this->_processHandle = 0;
      }

      void ExecutableFileClient::setInjectedParams(HookType key, bool val)
      {
        this->_paramHolder[key] = val;
      }

      void ExecutableFileClient::setServiceId(const QString& value)
      {
        this->_serviceId = value;
      }

    }
  }
}
