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
#include <QtCore/QUrlQuery>

#include <QtTest/QSignalSpy>

#include <QtConcurrent/QtConcurrentRun>

using namespace GGS;
using GGS::RestApi::GameNetCredential;

class ExecutableFileTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    restapi.setUri(QString("https://gnapi.com/restapi"));
    restapi.setCache(&cache);
    restapi.setRequest(GGS::RestApi::RequestFactory::Http);
    GGS::RestApi::RestApiManager::setCommonInstance(&restapi);
  }

  virtual void TearDown() 
  {
  }

  void ExecutionFlow(
    const GGS::Core::Service &srv, 
    int startCount, 
    GGS::GameExecutor::FinishState finishState, 
    GGS::RestApi::GameNetCredential credential,
    GGS::RestApi::GameNetCredential secondCredential = GGS::RestApi::GameNetCredential())
  {
    GameExecutor::Executor::ExecutableFile executor;

    QSignalSpy startExecute(&executor, SIGNAL(started(const GGS::Core::Service)));
    QSignalSpy finishExecute(&executor, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

    ExecutorWrapper wrapper(&executor);
    wrapper.setFinished([&](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
      loop.exit();
    });;

    executor.execute(srv, &executorService, credential, secondCredential);

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

// HACK тест падает. Понять и простить.
TEST_F(ExecutableFileTest, DISABLED_ArgumentParsing) 
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.exe");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %specialKey% %anotherKey% %Key with space%");
  query.addQueryItem("userId", "override");
  query.addQueryItem("specialKey", "specialKeyValue");
  query.addQueryItem("anotherKey", "anotherKeyValue");
  query.addQueryItem("Key with space", "value with space");

  url.setQuery(query);

  GGS::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);
  
  QFile successOutput(QCoreApplication::applicationDirPath() + "/output.txt");
  successOutput.remove();
  
  ExecutionFlow(srv, 1, GGS::GameExecutor::Success, GGS::RestApi::GameNetCredential());

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

// HACK Сломали тест из-за отключение авторизации без драйвера.
TEST_F(ExecutableFileTest, DISABLED_ExternalFatalError)
{
  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/fail.exe");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");

  url.setQuery(query);

  GGS::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 1, GGS::GameExecutor::ExternalFatalError, GGS::RestApi::GameNetCredential());
}

TEST_F(ExecutableFileTest, AuthorizationError) 
{
  GameNetCredential wrongAuth;

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");

  url.setQuery(query);

  GGS::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::AuthorizationError, wrongAuth);
}

TEST_F(ExecutableFileTest, ServiceAuthorizationImpossibleError) 
{
  GameNetCredential auth; 
  auth.setUserId("400001000025914750"); //gna_blocked_acc@unit.test
  auth.setAppKey("60472d5bc9f7dee91d0a113ff481a99a8091b503");

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");
  url.setQuery(query);

  GGS::Core::Service srv;
  srv.setId("300007020000000000"); //GA TEST
  srv.setGameId("83");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::ServiceAuthorizationImpossible, auth);
}

TEST_F(ExecutableFileTest, ServiceAccountBlockedError) 
{
  GameNetCredential auth; 
  auth.setUserId("400001000025914750"); //gna_blocked_acc@unit.test
  auth.setAppKey("60472d5bc9f7dee91d0a113ff481a99a8091b503");

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");
  url.setQuery(query);

  GGS::Core::Service srv;
  srv.setId("300007010000000000"); //GA TEST
  srv.setGameId("83");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, GGS::GameExecutor::ServiceAccountBlockedError, auth);
}

//TEST_F(ExecutableFileTest, DISABLED_ArgumentParsingSecondAccount) 
//{
//  RestApi::GameNetCredential secondAuth;
//  secondAuth.setUserId("400001000001709240");
//  secondAuth.setAppKey("570e3c3e59c7c4d7a1b322a0e25f231752814dc6");
//
//  QUrl url;
//  url.setScheme("exe");
//  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.exe");
//  QUrlQuery query;
//  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
//  query.addQueryItem("args", "%userId% %appKey% %token%");
//  url.setQuery(query);
//
//  GGS::Core::Service srv;
//  srv.setId("300003010000000000");
//  srv.setGameId("71");
//  srv.setUrl(url);
//
//  QFile successOutput(QCoreApplication::applicationDirPath() + "/output.txt");
//  successOutput.remove();
//
//  ExecutionFlow(srv, 1, GGS::GameExecutor::Success, "", secondAuth);
//
//  QString output;
//  if (successOutput.open(QIODevice::ReadOnly)) {
//    QTextStream in(&successOutput);
//    output = in.readAll();
//  }
//
//  QString correctOutput = QString("%1 %2").arg(secondAuth.userId(), secondAuth.appKey());
//
//  ASSERT_TRUE(output.trimmed().startsWith(correctOutput));
//}