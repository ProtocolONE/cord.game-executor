#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <Core/Service.h>

#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>

#include <QtTest/QSignalSpy>

#include <gtest/gtest.h>

using namespace P1;

class DownloadCustomFileTest : public ::testing::Test 
{
public:
  void SetUp() {
     srvHook.setArea(P1::Core::Service::Live);
     srvHook.setInstallPath(QCoreApplication::applicationDirPath());
     srvHook.setTorrentUrl(QUrl("http://files.gamenet.ru/update/bs/"));
  }

  P1::GameExecutor::FinishState executeHookCanStep(P1::Core::Service &service) 
  {
    GameExecutor::Hook::DownloadCustomFile hook1;
    QSignalSpy spy(&hook1, SIGNAL(canExecuteCompleted(const P1::Core::Service &, P1::GameExecutor::FinishState)));
    
    QEventLoop loop;
    QObject::connect(&hook1, SIGNAL(canExecuteCompleted(const P1::Core::Service &, P1::GameExecutor::FinishState)), &loop, SLOT(quit()), Qt::QueuedConnection);

    hook1.CanExecute(service);
    loop.exec();
    return spy.at(0).at(1).value<P1::GameExecutor::FinishState>();
  }

  P1::Core::Service srvHook;
};

TEST_F(DownloadCustomFileTest, Success) 
{
  QUrl url;
  QUrlQuery query;

  query.addQueryItem("downloadCustomFile", 
    "./launcher/serverinfo_back.xml,http://files.gamenet.ru/update/bs/,1,"\
    "./config/lastlogin.xml,http://files.gamenet.ru/update/bs/,1");
  
  url.setQuery(query);
  srvHook.setUrl(url);

  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/launcher");
  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/config");

  QFile fileInfo(QCoreApplication::applicationDirPath() + "/live/launcher/serverinfo_back.xml");
  fileInfo.open(QIODevice::WriteOnly);
  fileInfo.resize(0);
  fileInfo.close();

  QFile fileLogin(QCoreApplication::applicationDirPath() + "/live/config/lastlogin.xml");
  fileLogin.remove();
  
  ASSERT_EQ(P1::GameExecutor::Success, executeHookCanStep(srvHook));
  ASSERT_TRUE(fileInfo.exists());
  ASSERT_TRUE(fileInfo.size() > 0);
  ASSERT_TRUE(fileLogin.exists());
  ASSERT_TRUE(fileLogin.size() > 0);
}

TEST_F(DownloadCustomFileTest, SuccessNotOverrideMode) 
{
  QUrl url;
  QUrlQuery query;
  query.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,http://files.gamenet.ru/update/bs/,0");

  url.setQuery(query);
  srvHook.setUrl(url);

  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/launcher");

  QFile fileInfo(QCoreApplication::applicationDirPath() + "/live/launcher/serverinfo_back.xml");
  fileInfo.remove();
  fileInfo.open(QIODevice::WriteOnly);
  fileInfo.resize(0);
  fileInfo.close();
    
  ASSERT_EQ(P1::GameExecutor::Success, executeHookCanStep(srvHook));
  ASSERT_TRUE(fileInfo.exists());
  ASSERT_EQ(0, fileInfo.size());
}


TEST_F(DownloadCustomFileTest, UncorrectDomainFail) 
{
  QUrl url;
  QUrlQuery query;
  query.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,http://someVeryBadDomain.yes/update/bs/,1");
  url.setQuery(query);
  srvHook.setUrl(url);

  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/launcher");
    
  ASSERT_EQ(P1::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));
}

TEST_F(DownloadCustomFileTest, WrongArgsCount) 
{
  QUrl url;
  QUrlQuery query;
  query.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,http://files.gamenet.ru/update/bs/");
  url.setQuery(query);
  srvHook.setUrl(url);

  ASSERT_EQ(P1::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));

  query.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,1,someFile,3");
  url.setQuery(query);
  srvHook.setUrl(url);

  ASSERT_EQ(P1::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));
}