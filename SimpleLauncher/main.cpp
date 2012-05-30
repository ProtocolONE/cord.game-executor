/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/ExecutableFile.h>
#include <GameExecutor/Executor/WebLink.h>

#include <GameExecutor/Hook/RestoreResolution.h>
#include <GameExecutor/Hook/DisableIEDefalutProxy.h>
#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <Core/service.h>

#include <RestApi/RestApiManager>
#include <RestApi/RequestFactory>
#include <RestApi/GameNetCredential>
#include <RestApi/FakeCache>

#include <iostream>

#include <QtCore/QCoreApplication>
#include <QMetaType>

using GGS::RestApi::RestApiManager;
using GGS::RestApi::GameNetCredential;
using GGS::RestApi::FakeCache;

using GGS::Core::Service;
using GGS::GameExecutor::GameExecutorService;
using GGS::GameExecutor::Executor::ExecutableFile;
using GGS::GameExecutor::Executor::WebLink;
using GGS::GameExecutor::Hook::RestoreResolution;
using GGS::GameExecutor::Hook::DisableIEDefalutProxy;
using GGS::GameExecutor::Hook::DownloadCustomFile;


/*
Это тестовый проект, показывающий как можно пользоваться этим сервисом.
*/
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qRegisterMetaType<Service>("Core::Service");
    qRegisterMetaType<GGS::GameExecutor::FinishState>("GGS::GameExecutor::FinishState");
       

    QUrl url;
    url.addQueryItem("downloadCustomFile", "launcher/serverinfo_back.xml");
    url.addQueryItem("downloadCustomFileOverride", "1");


    Service srvHook;
    srvHook.setUrl(url);
    srvHook.setTorrentUrl(QUrl("http://files.gamenet.ru/update/bs"));
    srvHook.setInstallPath("c:\\tmp");
    
    DownloadCustomFile hook1;
    hook1.PreExecute(srvHook);

    int x;
    std::cin >> x;

    hook1.PostExecute(srvHook, GGS::GameExecutor::UnhandledRestApiError);
    
    

    RestApi::GameNetCredential auth;
    auth.setAppKey(QString("e8d6b0a31b408946334f23355ee2a0297f2758ac"));
    auth.setUserId(QString("400001000000065690"));

    RestApi::FakeCache cache;
    
    RestApi::RestApiManager restapi;
    restapi.setCache(&cache);
    restapi.setCridential(auth);
    restapi.setRequest(GGS::RestApi::RequestFactory::Http);

    WebLink cmd2;
    
    GameExecutorService execService;

    ExecutableFile cmd(&execService);
    cmd.setWorkingDirectory(QCoreApplication::applicationDirPath());
    
    execService.setRestApiManager(&restapi);
    execService.registerExecutor(&cmd);
    execService.registerExecutor(&cmd2);
    /*
    QUrl url;
    url.setScheme("exe");
    url.setPath("C:/GameNetLive/Games/mw2/mw2_bin/mw2.exe");
    url.addQueryItem("workingDir", "C:/GameNetLive/Games/mw2/mw2_bin/");
    
    url.addQueryItem("args", "%userId% %appKey% %token%");

    Service srv;
    srv.setId("300003010000000000");
    srv.setGameId("71");
    srv.setUrl(url);
    */
    
    // execService.execute(srv);
    /*
    QUrl url2("http://www.playga.ru");

    Service srv2;
    srv2.setId("300007010000000000");
    srv2.setGameId("81");
    srv2.setUrl(url2);

    execService.execute(srv2);
    */
    return a.exec();
}
