#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QStringList>

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

      ExecutableFileClient::ExecutableFileClient(QObject *parent) : QObject(parent)
      {
        DEBUG_LOG;
        SIGNAL_CONNECT_CHECK(connect(&this->_client, SIGNAL(connected()), this, SLOT(connectedToServer())));
        SIGNAL_CONNECT_CHECK(connect(&this->_client, SIGNAL(disconnected()), this, SLOT(disconnectedOrError())));
        SIGNAL_CONNECT_CHECK(connect(&this->_client, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(disconnectedOrError())));
        SIGNAL_CONNECT_CHECK(connect(&this->_client, SIGNAL(messageReceived(QString)), this, SLOT(messageFromServer(QString))));

        SIGNAL_CONNECT_CHECK(connect(&this->_executeFeatureWatcher, SIGNAL(finished()), this, SLOT(processFinished())));
      }

      ExecutableFileClient::~ExecutableFileClient()
      {
      }

      void ExecutableFileClient::setRestApiManager(RestApi::RestApiManager *restApiManager)
      {
        this->_restApiManager = restApiManager;
      }

      RestApi::RestApiManager *ExecutableFileClient::respApiManager()
      {
        return this->_restApiManager;
      }

      void ExecutableFileClient::exec()
      {
        DEBUG_LOG;
        this->_client.setName(this->_ipcName);
        this->_client.connectToServer();
      }

      void ExecutableFileClient::connectedToServer()
      {
        DEBUG_LOG;
      }

      void ExecutableFileClient::messageFromServer(QString message)
      {
        DEBUG_LOG << "'" << message << "'";

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

        GameNetCredential credential;
        credential.setUserId(params.at(4));
        credential.setAppKey(params.at(5));

        this->respApiManager()->setCridential(credential);

        QString dir = params.at(2);
        QString args = params.at(3);
        
        QString injectedDll;
        if (params.length() > 7)
          injectedDll = params.at(7);

        QTimer::singleShot(1000, this, SLOT(setUserActivity()));

        this->_executeFeature = QtConcurrent::run(
          this, 
          &ExecutableFileClient::startProcess, 
          exeFile, 
          params.at(2), 
          params.at(3), 
          injectedDll);

        this->_executeFeatureWatcher.setFuture(this->_executeFeature);
      }

      void ExecutableFileClient::setUserActivity()
      {
        DEBUG_LOG << "request";

        SetUserActivity *cmd = new SetUserActivity();

        cmd->setGameId(this->_gameId);  
        cmd->setLogout(0);

        SIGNAL_CONNECT_CHECK(connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(setUserActivityResult(GGS::RestApi::CommandBase::CommandResults))));

        this->respApiManager()->execute(cmd);
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
        DEBUG_LOG;

        this->_code = code;

        SetUserActivity *cmd = new SetUserActivity();

        cmd->setGameId(this->_gameId);
        cmd->setLogout(1);

        SIGNAL_CONNECT_CHECK(connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)), 
          this, SLOT(setUserActivityLogoutResult(GGS::RestApi::CommandBase::CommandResults))));

        this->respApiManager()->execute(cmd);
      }

      void ExecutableFileClient::setUserActivityLogoutResult(GGS::RestApi::CommandBase::CommandResults result)
      {
        DEBUG_LOG << "with result" << result;

        SetUserActivity *cmd = qobject_cast<SetUserActivity *>(QObject::sender());
        if (!cmd) {
          WARNING_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        cmd->deleteLater();

        emit this->exit(this->_code);
      }

      void ExecutableFileClient::disconnectedOrError()
      {
        emit this->exit(1);
      }

      void ExecutableFileClient::setIpcName(const QString& name)
      {
        this->_ipcName = name;
      }

      unsigned int ExecutableFileClient::startProcess(
        const QString& pathToExe, 
        const QString& workDirectory, 
        const QString& args, 
        const QString& dllPath /*= QString()*/)
      {
        QString commandLine = QString("\"%1\" %2").arg(pathToExe, args);

        QStringToWChar exe(pathToExe);
        QStringToWChar cmd(commandLine);
        QStringToWChar dir(workDirectory);

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb=sizeof(STARTUPINFO);

        if (!CreateProcessW(exe.data(), cmd.data(), 0, 0, FALSE, CREATE_SUSPENDED, NULL, dir.data(), &si, &pi))  {
          DEBUG_LOG << "Create process failed" << GetLastError();
          return 0xFFFFFFFF;
        }

        if (pi.hProcess == INVALID_HANDLE_VALUE) {
          DEBUG_LOG << "Create process invalid handle" << GetLastError();
          return 0xFFFFFFFE;
        }

        if (!dllPath.isEmpty()) {
          DEBUG_LOG << "Start inject";
          HMODULE hModule = GetModuleHandleW(L"Kernel32");
          QStringToWChar dll(dllPath);

          size_t len = (dll.size() + 1) * sizeof(wchar_t);
          void* pLibRemote = ::VirtualAllocEx(pi.hProcess, NULL, len, MEM_COMMIT, PAGE_READWRITE);
          DWORD iLen = 0;
          WriteProcessMemory(pi.hProcess, pLibRemote, static_cast<void*>(dll.data()), len, &iLen);

          HANDLE waitHandle = CreateEvent(NULL, FALSE, FALSE, L"Local\\QGNA_OVERLAY_EVENT");

          HANDLE hThread = CreateRemoteThread(
            pi.hProcess, 
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

        ResumeThread(pi.hThread);
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
      }
    }
  }
}
