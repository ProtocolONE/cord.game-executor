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
#include <RestApi/RequestFactory>
#include <RestApi/GameNetCredential>
#include <RestApi/HttpCommandRequest>
#include <RestApi/FakeCache>
#include <RestApi/Auth/GenericAuth>
#include <gtest/gtest.h>

#include <QtCore/QMetaType>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QtConcurrentRun>
#include <QtTest/QSignalSpy>

using namespace GGS;
using GGS::RestApi::GameNetCredential;

class ExecutableFileTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    auth.setAppKey(QString("599fd1beed859639ba44e7100383bb735f606c89"));
    auth.setUserId(QString("400001000001890150"));

    restapi.setUri(QString("https://api.gamenet.ru/restapi"));
    restapi.setCache(&cache);
    restapi.setCridential(auth);
    restapi.setRequest(GGS::RestApi::RequestFactory::Http);
    GGS::RestApi::RestApiManager::setCommonInstance(&restapi);
  }

  virtual void TearDown() 
  {
  }

  void ExecutionFlow(const GGS::Core::Service &srv, int startCount, GGS::GameExecutor::FinishState finishState, const QString workingPath = "")
  {
    GameExecutor::Executor::ExecutableFile executor;

    QSignalSpy startExecute(&executor, SIGNAL(started(const GGS::Core::Service)));
    QSignalSpy finishExecute(&executor, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

    ExecutorWrapper wrapper(&executor);
    wrapper.setFinished([&](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
      loop.exit();
    });;

    if (workingPath.length()) 
      executor.setWorkingDirectory(workingPath);

    executor.execute(srv, &executorService);

    loop.exec();
    //такая конструкция нужна, чтобы "доработать" события deleteLater
    while(loop.processEvents());

    ASSERT_EQ(startCount, startExecute.count());
    ASSERT_EQ(finishState, finishExecute.at(0).at(1).value<GGS::GameExecutor::FinishState>());
  }

  GGS::Core::Service service;
  
  GameExecutor::GameExecutorService executorService;

  QEventLoop loop;

  RestApi::RestApiManager restapi;
  RestApi::HttpCommandRequest request;
  RestApi::FakeCache cache;
  RestApi::GameNetCredential auth;
};

TEST_F(ExecutableFileTest, Scheme) 
{
  GameExecutor::Executor::ExecutableFile executor;
  ASSERT_EQ("exe", executor.scheme());
}

TEST_F(ExecutableFileTest, Success) 
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %token% %login%");
  
  GGS::Core::Service srv;
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

  GGS::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);
  
  QFile successOutput(QCoreApplication::applicationDirPath() + "/output.txt");
  successOutput.remove();
  
  ExecutionFlow(srv, 1, GGS::GameExecutor::Success);

  QString output;
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

  GGS::Core::Service srv;
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

  GGS::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::ExternalFatalError, "WrongPathToLauncer.Exe");
}

TEST_F(ExecutableFileTest, AuthorizationError) 
{
  GameNetCredential wrongAuth;
  restapi.setCridential(wrongAuth);

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %appKey% %token%");

  GGS::Core::Service srv;
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

  restapi.setCridential(auth);

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  url.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  url.addQueryItem("args", "%userId% %appKey% %token%");

  GGS::Core::Service srv;
  srv.setId("300007020000000000"); //GA TEST
  srv.setGameId("83");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::ServiceAccountBlockedError);
}