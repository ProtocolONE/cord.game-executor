#include <GameExecutor/GameExecutorService.h>

#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QWinEventNotifier>
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QSysInfo>

#include <RestApi/GameNetCredential.h>

#include <Core/System/ProcessTools.h>

#include <sstream>

using P1::RestApi::GameNetCredential;

namespace P1{
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
        const QString& workDirectory)
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

        if (!CreateProcessW(exe.data(), cmd.data(), 0, 0, FALSE, CREATE_SUSPENDED, NULL, dir.data(), &si, &pi)) {
          DEBUG_LOG << "Create process failed" << GetLastError();
          emit this->finished(Fail);
          return;
        }

        if (pi.hProcess == INVALID_HANDLE_VALUE) {
          DEBUG_LOG << "Create process invalid handle" << GetLastError();
          emit this->finished(Fail);
          return;
        }

        QWinEventNotifier *monitor = new QWinEventNotifier(pi.hProcess, this);
        QObject::connect(monitor, &QWinEventNotifier::activated, 
          this, &ExecutableFileClient::handleActivated);

        this->_processHandle = pi.hProcess;
        this->_threadHandle = pi.hThread;

        ResumeThread(pi.hThread);
      }

      void ExecutableFileClient::stopProcess()
      {
        if (this->_processHandle != NULL)
          P1::Core::System::ProcessTools::killProcessTree(::GetProcessId(this->_processHandle), 0);
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
