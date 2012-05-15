/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/GameExecutorServiceWrapper.h>
#include <GameExecutor/Executor/ExecutorWrapper.h>

#include <GameExecutor/Enum.h>
#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile.h>

#include <Core/Service>

#include <RestApi/RestApiManager>
#include <RestApi/GameNetCredential>
#include <RestApi/HttpCommandRequest>
#include <RestApi/FakeCache>

#include <gtest/gtest.h>

#include <QtCore/QMetaType>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QtConcurrentRun>
#include <QtTest/QSignalSpy>

using namespace GGS;

class ExecutableFileTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    auth.setAppKey(QString("e8d6b0a31b408946334f23355ee2a0297f2758ac"));
    auth.setUserId(QString("400001000000065690"));

    request.setCache(&cache);
    
    restapi.setUri(QString("https://api.gamenet.ru/restapi"));
    restapi.setCridential(auth);
    restapi.setRequest(&request);

    executorService.setRestApiManager(&restapi);
  }

  virtual void TearDown() 
  {
  }

  void ExecutionFlow(const Core::Service &srv, int startCount, GGS::GameExecutor::FinishState finishState)
  {
    QSignalSpy startExecute(&executor, SIGNAL(started(const Core::Service)));
    QSignalSpy finishExecute(&executor, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)));

    ExecutorWrapper wrapper(&executor);
    wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
      loop.exit();
    });;

    //Очень важно запускать именно через отдельный поток, т.к. в реальном использовании это именно так.
    QtConcurrent::run(&executor, &GGS::GameExecutor::Executor::ExecutableFile::execute, srv, &executorService);

    loop.exec();

    ASSERT_EQ(startCount, startExecute.count());
    ASSERT_EQ(finishState, finishExecute.at(0).at(1).value<GGS::GameExecutor::FinishState>());
  }

  Core::Service service;
  
  GameExecutor::Executor::ExecutableFile executor;
  GameExecutor::GameExecutorService executorService;

  QEventLoop loop;

  RestApi::RestApiManager restapi;
  RestApi::HttpCommandRequest request;
  RestApi::FakeCache cache;
  RestApi::GameNetCredential auth;
};

TEST_F(ExecutableFileTest, Scheme) 
{
  ASSERT_EQ("exe", executor.scheme());
}

TEST_F(ExecutableFileTest, Success) 
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %token% %login%");
  
  Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 1, GGS::GameExecutor::Success);
}

TEST_F(ExecutableFileTest, ArgumentParsing) 
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.exe");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %appKey% %specialKey% %anotherKey% %Key with space%");
  url.addQueryItem("userId", "override");
  url.addQueryItem("specialKey", "specialKeyValue");
  url.addQueryItem("anotherKey", "anotherKeyValue");
  url.addQueryItem("Key with space", "value with space");

  Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 1, GGS::GameExecutor::Success);

  QString output;
  QString filePath = QCoreApplication::applicationDirPath() + "/output.txt";
  QFile successOutput(filePath);
  if (successOutput.open(QIODevice::ReadOnly)) {
    QTextStream in(&successOutput);
    output = in.readAll();
  }
  
  QString correctOutput = QString("%1 %2 %3 %4 %5")
                          .arg("override").arg(auth.appKey()).arg("specialKeyValue")
                          .arg("anotherKeyValue").arg("value with space");

  ASSERT_EQ(correctOutput, output.trimmed());
}

TEST_F(ExecutableFileTest, ExternalFatalError) 
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/fail.exe");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %appKey% %token%");

  Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 1, GGS::GameExecutor::ExternalFatalError);
}

TEST_F(ExecutableFileTest, ExternalFatalErrorIfLauncerExeFailed) 
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %token%");

  Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  executor.setWorkingDirectory("WrongPathToLauncer.Exe");

  ExecutionFlow(srv, 0, GGS::GameExecutor::ExternalFatalError);
}

TEST_F(ExecutableFileTest, AuthorizationError) 
{
  GameNetCredential wrongAuth;
  executorService.respApiManager()->setCridential(wrongAuth);

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %appKey% %token%");

  Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::AuthorizationError);
}

TEST_F(ExecutableFileTest, ServiceAccountBlockedError) 
{
  GameNetCredential auth; 
  auth.setUserId("400001000025914750"); //gna_blocked_acc@unit.test
  auth.setAppKey("60472d5bc9f7dee91d0a113ff481a99a8091b503");

  executorService.respApiManager()->setCridential(auth);

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %appKey% %token%");

  Core::Service srv;
  srv.setId("300007020000000000"); //GA TEST
  srv.setGameId("83");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::ServiceAccountBlockedError);
}