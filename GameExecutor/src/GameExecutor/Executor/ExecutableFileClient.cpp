/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Extension.h>

#include <GameExecutor/Executor/ExecutableFileClient.h>
#include <GameExecutor/Executor/AppInitPatch.h>

#include <GameNetAuthHost/AuthWriter.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QWinEventNotifier>
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QSysInfo>

#include <RestApi/GameNetCredential>
#include <RestApi/Commands/User/SetUserActivity>

#include <Common/ConfigReader.h>

#include <sstream>

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
        , _authWriter(NULL)
        , _executorService(NULL)
      {
      }

      ExecutableFileClient::~ExecutableFileClient()
      {
        this->stopProcess();
        this->closeAuth();
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

        {
          ProcessHandleCheckExtension *extension = reinterpret_cast<ProcessHandleCheckExtension *>(
            this->_executorService->extension(ExtensionTypes::ProcessHandleCheck));

          if (extension->get()(pi.dwProcessId, this->_processHandle))
            this->initAuth(pi.dwProcessId);
        }

        bool cmdFix = false;
        bool sphFix = false;
        bool x64Helper = false;
        bool winHook = false;
        uint8_t winVersion = 0;

        if (this->_paramHolder.count(CommandLineCheck) && !this->isAuthSdkEnabled())
          cmdFix = this->_paramHolder[CommandLineCheck];

        if (this->_paramHolder.count(SpeedHackCheck))
          sphFix = this->_paramHolder[SpeedHackCheck];

        if (this->_paramHolder.count(Need64Load))
          x64Helper = this->_paramHolder[Need64Load];

        // Данный код используется для защиты от AppInit_Dll.
        // В случае 32 битов передается хэндл процесса, 64 бита - пид процесса.
        AppInitDllPatchExtension *appInitDllPatch = reinterpret_cast<AppInitDllPatchExtension *>(
          this->_executorService->extension(ExtensionTypes::AppInitDllPatch));
        if (appInitDllPatch) {
          // UNDONE Урать условие - защита должна быть всегда
          appInitDllPatch->get()(x64Helper, x64Helper ? pi.dwProcessId : (DWORD)pi.hProcess);
        }

        if (this->_paramHolder.count(WinHookDefendCheck))
          winHook = this->_paramHolder[WinHookDefendCheck];

        QSysInfo::WinVersion winVer = QSysInfo::windowsVersion();

        switch (winVer)
        {
        case QSysInfo::WV_XP:
          winVersion = 0;
          break;
        case QSysInfo::WV_VISTA:
          winVersion = 1;
          break;
        case QSysInfo::WV_WINDOWS7:
          winVersion = 2;
          break;
        case QSysInfo::WV_WINDOWS8:
          winVersion = 3;
          break;
        case QSysInfo::WV_WINDOWS8_1:
          winVersion = 4;
          break;
        case QSysInfo::WV_WINDOWS10:
          winVersion = 5;
          break;
        default:
          winVersion = 6;
          break;
        }
        
        std::vector<std::wstring> toLoad;
        toLoad.push_back(L"user32.dll");

        if (!nvidia.isEmpty())
          toLoad.push_back(nvidia.toStdWString());
        
        GGS::GameExecutorHelper::ConfigReader cfg;

        cfg.setPid(pi.dwProcessId);
        cfg.setCommandLineFlag(cmdFix);
        cfg.setSpeedHackFlag(sphFix);
        cfg.setWinHookDefeneder(winHook);
        cfg.setWindowsVersion(winVersion);
        cfg.setStrings(toLoad);
        cfg.setServiceId(this->_serviceId.toStdWString());
        cfg.write();

        // Данный код используется для инжекта в стартуемый прцесс.
        // В случае 32 битов передается хэндл процесса, 64 бита - пид процесса.
        InjectDllExtension *extension = reinterpret_cast<InjectDllExtension *>(
          this->_executorService->extension(ExtensionTypes::InjectDll));

        if (!injectedDll.isEmpty()) {
          extension->get()(x64Helper, x64Helper ? pi.dwProcessId : (DWORD)pi.hProcess, injectedDll, QString("Local\\QGNA_OVERLAY_EVENT"));
        }

        QString dllHelper;

#ifdef _DEBUG
        dllHelper = x64Helper ? "/GameExecutorHelperX64d.dll" : "/GameExecutorHelperX86d.dll"; 
#else
        dllHelper = x64Helper ? "/GameExecutorHelperX64.dll" : "/GameExecutorHelperX86.dll";
#endif

        QString executorHelper = QCoreApplication::applicationDirPath() + dllHelper;

        if (cmdFix)
          this->_shareArgs(pi.dwProcessId, this->_processHandle);

        extension->get()(x64Helper, x64Helper ? pi.dwProcessId : (DWORD)pi.hProcess, executorHelper, QString("Local\\QGNA_OVERLAY_EVENT2"));

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

      void ExecutableFileClient::initAuth(const quint32 pid)
      {
        ProcessHandleCheckExtension *extension = reinterpret_cast<ProcessHandleCheckExtension *>(
          this->_executorService->extension(ExtensionTypes::ProcessHandleCheck));

        if (!extension->get()(pid, this->_processHandle))
          return;

        if (!this->isAuthSdkEnabled())
          return;

        std::stringstream ss;
        ss << "Local\\GameNet_HelperInfo_" << pid << "_";
        this->_authWriter = new GameNetAuthHost::AuthWriter();

        const QString & userId = this->_authParam[UserIdData];
        const QString & appKey = this->_authParam[AppKeyData];
        const QString & token = this->_authParam[TokenData];

        if (!extension->get()(pid, this->_processHandle))
          return;

        this->_authWriter->write(ss.str(), userId.toStdString(), appKey.toStdString(), token.toStdString(), "");
      }

      void ExecutableFileClient::closeAuth()
      {
        delete this->_authWriter;
        this->_authWriter = NULL;
      }

      void ExecutableFileClient::setAuthParam(AuthData key, const QString & value)
      {
        this->_authParam[key] = value;
      }

      bool ExecutableFileClient::isAuthSdkEnabled()
      {
        if (!this->_authParam.count(UseAuthSdk))
          return false;

        return this->_authParam[UseAuthSdk] == "1";
      }

      void ExecutableFileClient::setService(GGS::GameExecutor::GameExecutorService * srv)
      {
        this ->_executorService = srv;
      }
    }
  }
}
