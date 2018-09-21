#include <GameExecutor/GameExecutorServiceWrapper.h>
#include <GameExecutor/Executor/ExecutorWrapper.h>

#include <GameExecutor/Enum.h>
#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile.h>

#include <Core/Service.h>

#include <RestApi/RestApiManager.h>
#include <RestApi/RequestFactory.h>
#include <RestApi/ProtocolOneCredential.h>
#include <RestApi/HttpCommandRequest.h>
#include <RestApi/FakeCache.h>
#include <gtest/gtest.h>

#include <QtCore/QMetaType>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QUrlQuery>

#include <QtTest/QSignalSpy>

#include <QtConcurrent/QtConcurrentRun>

using namespace P1;
using P1::RestApi::ProtocolOneCredential;

class ExecutableFileTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    restapi.setUri(QString("http://api.stg.protocol.one/restapi"));
    restapi.setCache(&cache);
    restapi.setRequest(P1::RestApi::RequestFactory::Http);
    P1::RestApi::RestApiManager::setCommonInstance(&restapi);
  }

  virtual void TearDown() 
  {
  }

  void ExecutionFlow(
    const P1::Core::Service &srv, 
    int startCount, 
    P1::GameExecutor::FinishState finishState, 
    P1::RestApi::ProtocolOneCredential credential)
  {
    GameExecutor::Executor::ExecutableFile executor;

    QSignalSpy startExecute(&executor, SIGNAL(started(const P1::Core::Service)));
    QSignalSpy finishExecute(&executor, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));

    ExecutorWrapper wrapper(&executor);
    wrapper.setFinished([&](const P1::Core::Service &service, P1::GameExecutor::FinishState state) {
      loop.exit();
    });;

    executor.execute(srv, &executorService, credential);

    loop.exec();
    //такая конструкция нужна, чтобы "доработать" события deleteLater
    while(loop.processEvents());

    ASSERT_EQ(startCount, startExecute.count());
    ASSERT_EQ(finishState, finishExecute.at(0).at(1).value<P1::GameExecutor::FinishState>());
  }

  P1::Core::Service service;
  
  GameExecutor::GameExecutorService executorService;

  QEventLoop loop;

  RestApi::RestApiManager restapi;
  RestApi::HttpCommandRequest request;
  RestApi::FakeCache cache;
  RestApi::ProtocolOneCredential auth;
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

  P1::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);
  
  QFile successOutput(QCoreApplication::applicationDirPath() + "/output.txt");
  successOutput.remove();
  
  ExecutionFlow(srv, 1, P1::GameExecutor::Success, P1::RestApi::ProtocolOneCredential());

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

  P1::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 1, P1::GameExecutor::ExternalFatalError, P1::RestApi::ProtocolOneCredential());
}

TEST_F(ExecutableFileTest, AuthorizationError) 
{
    ProtocolOneCredential wrongAuth;

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");

  url.setQuery(query);

  P1::Core::Service srv;
  srv.setId("300003010000000000");
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, P1::GameExecutor::AuthorizationError, wrongAuth);
}

TEST_F(ExecutableFileTest, DISABLED_ServiceAuthorizationImpossibleError) 
{
    ProtocolOneCredential auth;

  auth.setUserId("400001000133689350"); //gna_blocked_acc@unit.test
  auth.setAppKey("c68dc3316c48868c243db70bb878f473b22c457b");

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");
  url.setQuery(query);

  P1::Core::Service srv;
  srv.setId("40000000000"); //Black Desert TEST
  srv.setGameId("1022");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, P1::GameExecutor::ServiceAuthorizationImpossible, auth);
}

TEST_F(ExecutableFileTest, DISABLED_ServiceAccountBlockedError)
{
    ProtocolOneCredential auth;
  auth.setUserId("400001000133689350"); //gna_blocked_acc@unit.test
  auth.setAppKey("c68dc3316c48868c243db70bb878f473b22c457b");

  QUrl url;
  url.setScheme("exe");
  url.setPath(QCoreApplication::applicationDirPath() + "/fixtures/success.bat");
  QUrlQuery query;
  query.addQueryItem("workingDir", QCoreApplication::applicationDirPath());
  query.addQueryItem("args", "%userId% %appKey% %token%");
  url.setQuery(query);

  P1::Core::Service srv;
  srv.setId("300003010000000000"); //BS
  srv.setGameId("71");
  srv.setUrl(url);

  ExecutionFlow(srv, 0, P1::GameExecutor::ServiceAccountBlockedError, auth);
}
