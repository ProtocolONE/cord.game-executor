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

#include <RestApi/GameNetCredential>
#include <RestApi/Commands/User/SetUserActivity>

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
        const QString& injectedDll,
        const QString& injectedDll2)
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

        // INFO Инжектнем первой, ибо не умеем ждать пока загрузка пройдет успешно.
        if (!nvidia.isEmpty())
          this->injectDll(this->_processHandle, nvidia);

        if (!injectedDll.isEmpty())
          this->injectDll(this->_processHandle, injectedDll, QString("Local\\QGNA_OVERLAY_EVENT"));

        // UNDONE рефакторнуть это как-то. Например грузить все дополнительные дллки через общий хелпер.
        if (!injectedDll2.isEmpty()) {
          this->_shareArgs(pi.dwProcessId, this->_processHandle);
          this->injectDll(this->_processHandle, injectedDll2, QString("Local\\QGNA_OVERLAY_EVENT2"));
          this->_deleteSharedArgs();
        }

        ResumeThread(pi.hThread);

        // INFO этот код важен. Необходимо подождать пока проинициализируется user32.dll и только потом восстанавливать
        // реестр.
        Sleep(500);
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

    }
  }
}
