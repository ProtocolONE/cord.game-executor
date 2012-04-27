#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>

#include <RestApi/GameNetCredential>
#include <RestApi/Commands/User/SetUserActivity>

#include <windows.h>
#include <tchar.h>
#include <psapi.h>

using GGS::RestApi::GameNetCredential;
using GGS::RestApi::Commands::User::SetUserActivity;

namespace GGS{
  namespace GameExecutor{
    namespace Executor {
      ExecutableFileClient::ExecutableFileClient(QObject *parent) : QObject(parent), _finishPassed(false)
      {
        qDebug() << __FUNCTION__;

        this->_client.setName(QString(GAMEEXECUTOR_IPC_CHANNEL));

        connect(&this->_client, SIGNAL(connected()), this, SLOT(connectedToServer()));
        connect(&this->_client, SIGNAL(messageReceived(QString)), this, SLOT(messageFromServer(QString)));

        connect(&this->_gameProcess, SIGNAL(started()), this, SLOT(processStarted()));
        connect(&this->_gameProcess, SIGNAL(finished(int, QProcess::ExitStatus)), 
                this, SLOT(processFinished(int, QProcess::ExitStatus)));
        connect(&this->_gameProcess, SIGNAL(error(QProcess::ProcessError)), 
                this, SLOT(processError(QProcess::ProcessError)));
      }

      ExecutableFileClient::~ExecutableFileClient()
      {

      }

      void ExecutableFileClient::setRestApiManager( RestApi::RestApiManager *restApiManager )
      {
        this->_restApiManager = restApiManager;
      }

      RestApi::RestApiManager *ExecutableFileClient::respApiManager()
      {
        return this->_restApiManager;
      }

      void ExecutableFileClient::exec()
      {
        qDebug() << __FUNCTION__;
        this->_client.connectToServer();
      }

      void ExecutableFileClient::connectedToServer()
      {
        qDebug() << __LINE__ << __FUNCTION__;
      }

      void ExecutableFileClient::messageFromServer( QString message )
      {
        qDebug() << __LINE__ << __FUNCTION__ << "'" << message << "'";

        QStringList params = message.split("|", QString::KeepEmptyParts, Qt::CaseInsensitive);
        if (params.at(0) != "start") {
          qCritical() << __LINE__ << __FUNCTION__;
          emit this->exit(Fail);
          return;
        }

        QString exeFile = params.at(1);

        if (!QFile::exists(exeFile)) {
          qCritical() << __LINE__ << __FUNCTION__ "'" << exeFile << "'";
          emit this->exit(Fail);
          return;
        }

        bool gameIdCastResult = false;
        this->_gameId = params.at(6).toInt(&gameIdCastResult);

        if (false == gameIdCastResult) {
          qCritical() << __LINE__ << __FUNCTION__ << params.at(6);
          emit this->exit(Fail);
          return;
        }
        
        GameNetCredential credential;
        credential.setUserId(params.at(4));
        credential.setAppKey(params.at(5));

        this->respApiManager()->setCridential(credential);

        this->_gameProcess.setWorkingDirectory(params.at(2));

        QString args = params.at(3);
        this->_gameProcess.start(exeFile, args.split(" "));

        this->_client.sendMessage("Starting");  
      }

      void ExecutableFileClient::processError(QProcess::ProcessError error)
      {
        if (this->_finishPassed) 
          return;

        this->_finishPassed = true;

        qDebug() << __LINE__ << __FUNCTION__ << error << this->_gameProcess.exitCode();

        if (error == QProcess::FailedToStart) {
          this->setUserActivityLogout(Fail);
        }
      }

      void ExecutableFileClient::processStarted()
      {
        qDebug() << __LINE__ << __FUNCTION__ << this->_gameProcess.pid()->dwProcessId;

        this->_pid = this->_gameProcess.pid();

        QTimer::singleShot(1000, this, SLOT(setUserActivity()));
        QTimer::singleShot(1000, this, SLOT(checkProcessIsAlive()));
      }

      void ExecutableFileClient::checkProcessIsAlive()
      {
        DWORD processIdArray[1024];
        DWORD bytesReturned ;
        
        //UNDONE Есть непроверенная проблема. Возможно, функция EnumProcesses не будет возвращать PID для х64 
        //процессов, если она вызывается из х32 процесса. С другой стороны у нас нет сейчас х64 игр.
        //http://msdn.microsoft.com/en-us/library/windows/desktop/ms682629(v=vs.85).aspx
        if (!EnumProcesses(processIdArray, sizeof(processIdArray), &bytesReturned)) {
          qFatal("EnumProcesses fail");
          return;
        }

        bool processExists = false;
        DWORD processCount = bytesReturned / sizeof(DWORD);
        for (unsigned int i = 0; i < processCount; i++) {
          if(processIdArray[i] == this->_pid->dwProcessId ) {
            processExists = true;
            break;
          }
        }

        if (processExists) {
          QTimer::singleShot(5000, this, SLOT(checkProcessIsAlive()));
          return;
        } 

        qDebug() << __LINE__ << __FUNCTION__;

        if (this->_finishPassed) 
          return;

        this->_finishPassed = true;

        this->setUserActivityLogout(Success);
      }

      void ExecutableFileClient::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
      {
        if (this->_finishPassed) 
          return;

        this->_finishPassed = true;

        int realExitCode = 
          (exitStatus == QProcess::CrashExit && static_cast<unsigned int>(exitCode) != 0xC0000005) ? Fail : Success;

        qDebug() << __LINE__ << __FUNCTION__ << exitStatus << static_cast<unsigned int>(exitCode) << realExitCode;

        this->setUserActivityLogout(realExitCode);
      }

      void ExecutableFileClient::setUserActivity()
      {
        qDebug() << __LINE__ << __FUNCTION__;

        SetUserActivity *cmd = new SetUserActivity();

        cmd->setGameId(this->_gameId);  
        cmd->setLogout(0);

        connect(cmd, SIGNAL(result(GGS::RestApi::CommandBaseInterface::CommandResults)), 
                this, SLOT(setUserActivityResult(GGS::RestApi::CommandBaseInterface::CommandResults)));
        
        this->respApiManager()->execute(cmd);
      }

      void ExecutableFileClient::setUserActivityResult(GGS::RestApi::CommandBaseInterface::CommandResults result)
      {
        SetUserActivity *cmd = qobject_cast<SetUserActivity *>(QObject::sender());
        if (!cmd) {
          qWarning() << __LINE__ << __FUNCTION__;
          return;
        }

        cmd->deleteLater();

        if (result != CommandBaseInterface::NoError) {
          qWarning() << __LINE__ << __FUNCTION__ << result << cmd->getGenericErrorMessageCode();
          QTimer::singleShot(300000, this, SLOT(setUserActivity())); //default timeOut is 5 minutes;
          return;
        }

        int timeOut = cmd->getTimeout();
        if (timeOut <= 0) {
          qWarning() <<  "setUserActivityResult wrong timeOut " << timeOut;
          QTimer::singleShot(300000, this, SLOT(setUserActivity()));
          return;  
        }

        QTimer::singleShot(timeOut * 1000, this, SLOT(setUserActivity()));

        qDebug() << __LINE__ << __FUNCTION__;
      }

      void ExecutableFileClient::setUserActivityLogout(int code)
      {
        qDebug() << __LINE__ << __FUNCTION__;

        this->_code = code;

        SetUserActivity *cmd = new SetUserActivity();

        cmd->setGameId(this->_gameId);
        cmd->setLogout(1);

        connect(cmd, SIGNAL(result(GGS::RestApi::CommandBaseInterface::CommandResults)), 
                this, SLOT(setUserActivityLogoutResult(GGS::RestApi::CommandBaseInterface::CommandResults)));

        this->respApiManager()->execute(cmd);
      }

      void ExecutableFileClient::setUserActivityLogoutResult(GGS::RestApi::CommandBaseInterface::CommandResults result)
      {
         qDebug() << __LINE__ << __FUNCTION__ << result;

         SetUserActivity *cmd = qobject_cast<SetUserActivity *>(QObject::sender());
         if (!cmd) {
           qWarning() << __LINE__ << __FUNCTION__;
           return;
         }

         cmd->deleteLater();

         emit this->exit(this->_code);
      }
    }
  }
}
