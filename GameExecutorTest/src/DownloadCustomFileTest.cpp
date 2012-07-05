/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <Core/Service>

#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>

#include <QtTest/QSignalSpy>

#include <gtest/gtest.h>

using namespace GGS;

class DownloadCustomFileTest : public ::testing::Test 
{
public:
  void SetUp() {
     srvHook.setArea(Core::Service::Live);
     srvHook.setInstallPath(QCoreApplication::applicationDirPath());
     srvHook.setTorrentUrl(QUrl("http://files.gamenet.ru/update/bs/"));
  }

  GGS::GameExecutor::FinishState executeHookCanStep(Core::Service &service) 
  {
    GameExecutor::Hook::DownloadCustomFile hook1;
    QSignalSpy spy(&hook1, SIGNAL(canExecuteCompleted(GGS::GameExecutor::FinishState)));
    
    QEventLoop loop;
    QObject::connect(&hook1, SIGNAL(canExecuteCompleted(GGS::GameExecutor::FinishState)), &loop, SLOT(quit()), Qt::QueuedConnection);

    hook1.CanExecute(service);
    loop.exec();
    return spy.at(0).at(0).value<GGS::GameExecutor::FinishState>();
  }

  Core::Service srvHook;
};

TEST_F(DownloadCustomFileTest, Success) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", 
    "./launcher/serverinfo_back.xml,http://files.gamenet.ru/update/bs/,1,"\
    "./config/lastlogin.xml,http://files.gamenet.ru/update/bs/,1");
  
  srvHook.setUrl(url);

  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/launcher");
  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/config");

  QFile fileInfo(QCoreApplication::applicationDirPath() + "/live/launcher/serverinfo_back.xml");
  fileInfo.open(QIODevice::WriteOnly);
  fileInfo.resize(0);
  fileInfo.close();

  QFile fileLogin(QCoreApplication::applicationDirPath() + "/live/config/lastlogin.xml");
  fileLogin.remove();
  
  ASSERT_EQ(GGS::GameExecutor::Success, executeHookCanStep(srvHook));
  ASSERT_TRUE(fileInfo.exists());
  ASSERT_TRUE(fileInfo.size() > 0);
  ASSERT_TRUE(fileLogin.exists());
  ASSERT_TRUE(fileLogin.size() > 0);
}

TEST_F(DownloadCustomFileTest, SuccessNotOverrideMode) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,http://files.gamenet.ru/update/bs/,0");

  srvHook.setUrl(url);

  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/launcher");

  QFile fileInfo(QCoreApplication::applicationDirPath() + "/live/launcher/serverinfo_back.xml");
  fileInfo.remove();
  fileInfo.open(QIODevice::WriteOnly);
  fileInfo.resize(0);
  fileInfo.close();
    
  ASSERT_EQ(GGS::GameExecutor::Success, executeHookCanStep(srvHook));
  ASSERT_TRUE(fileInfo.exists());
  ASSERT_EQ(0, fileInfo.size());
}


TEST_F(DownloadCustomFileTest, UncorrectDomainFail) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,http://someVeryBadDomain.yes/update/bs/,1");

  srvHook.setUrl(url);

  QDir().mkpath(QCoreApplication::applicationDirPath() + "/live/launcher");
    
  ASSERT_EQ(GGS::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));
}

TEST_F(DownloadCustomFileTest, WrongArgsCount) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,http://files.gamenet.ru/update/bs/");

  srvHook.setUrl(url);

  ASSERT_EQ(GGS::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));

  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,1,someFile,3");
  srvHook.setUrl(url);

  ASSERT_EQ(GGS::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));
}