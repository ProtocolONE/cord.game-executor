/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/Hook/BannerDownload.h>

#include <Core/Service>

#include <RestApi/RestApiManager>
#include <RestApi/FakeCache>

#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>

#include <QtTest/QSignalSpy>

#include <gtest/gtest.h>

class BannerDownloadTest : public ::testing::Test 
{
public:
  void SetUp() {
    srvHook.setId("300009010000000000");
    srvHook.setArea(GGS::Core::Service::Live);
    srvHook.setInstallPath(QCoreApplication::applicationDirPath());

    _restApi.setCache(&_cache);
    _restApi.setRequest(GGS::RestApi::RequestFactory::Http);

    GGS::RestApi::RestApiManager::setCommonInstance(&_restApi);
  }

  void TearDown() {
  }

  GGS::GameExecutor::FinishState executeHookPreStep(GGS::Core::Service &service) 
  {
    GGS::GameExecutor::Hook::BannerDownload hook1;
    QSignalSpy spy(&hook1, SIGNAL(preExecuteCompleted(const GGS::Core::Service &, GGS::GameExecutor::FinishState)));

    QEventLoop loop;
    QObject::connect(&hook1, SIGNAL(preExecuteCompleted(const GGS::Core::Service &, GGS::GameExecutor::FinishState)), &loop, SLOT(quit()), Qt::QueuedConnection);

    hook1.PreExecute(service);
    loop.exec();
    return spy.at(0).at(1).value<GGS::GameExecutor::FinishState>();
    //return GGS::GameExecutor::CanExecutionHookBreak;
  }

  GGS::Core::Service srvHook;
  GGS::RestApi::RestApiManager _restApi;
  GGS::RestApi::FakeCache _cache;
};

TEST_F(BannerDownloadTest, Success) 
{
  ASSERT_EQ(GGS::GameExecutor::Success, executeHookPreStep(srvHook));
}