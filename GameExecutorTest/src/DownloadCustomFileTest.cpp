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
#include <Windows.h>

using namespace GGS;

class DownloadCustomFileTest : public ::testing::Test 
{
public:
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
};

TEST_F(DownloadCustomFileTest, Success) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml");
  url.addQueryItem("downloadCustomFileOverride", "1");
  url.addQueryItem("downloadCustomFileUrl", "http://files.gamenet.ru/update/bs/");
  //http://files.gamenet.ru/update/bs/live/launcher/serverinfo_back.xml
  Core::Service srvHook;
  srvHook.setUrl(url);
  srvHook.setArea(Core::Service::Live);
  srvHook.setTorrentUrl(QUrl("http://files.gamenet.ru/update/bs/"));
  
  QString basePath = QCoreApplication::applicationDirPath();
  QDir().mkpath(basePath + "/launcher");
  
  srvHook.setInstallPath(basePath);

  QFile file(basePath + "./live/launcher/serverinfo_back.xml");
  file.remove();

  ASSERT_EQ(GGS::GameExecutor::Success, executeHookCanStep(srvHook));
  ASSERT_TRUE(file.exists());
  ASSERT_TRUE(file.size() > 0);
}

TEST_F(DownloadCustomFileTest, UncorrectDomainFail) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml");
  url.addQueryItem("downloadCustomFileOverride", "1");

  Core::Service srvHook;
  srvHook.setUrl(url);
  srvHook.setArea(Core::Service::Live);
  srvHook.setTorrentUrl(QUrl("http://someVeryBadDomain.yes/update/bs/"));

  QString basePath = QCoreApplication::applicationDirPath();
  QDir().mkpath(basePath + "/launcher");

  srvHook.setInstallPath(basePath);

  ASSERT_EQ(GGS::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));
}

TEST_F(DownloadCustomFileTest, LockedFileFail) 
{
  QUrl url;
  url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml");
  url.addQueryItem("downloadCustomFileOverride", "1");

  Core::Service srvHook;
  srvHook.setUrl(url);
  srvHook.setArea(Core::Service::Live);
  srvHook.setTorrentUrl(QUrl("http://someVeryBadDomain.yes/update/bs/"));

  QString basePath = QCoreApplication::applicationDirPath();
  QDir().mkpath(basePath + "/launcher");

  srvHook.setInstallPath(basePath);

  QString finalFilePath = basePath + "./launcher/serverinfo_back.xml";

  HANDLE hFile = CreateFile(finalFilePath.toStdWString().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

  GameExecutor::Hook::DownloadCustomFile hook1;
  ASSERT_EQ(GGS::GameExecutor::CanExecutionHookBreak, executeHookCanStep(srvHook));

  CloseHandle(hFile);
}