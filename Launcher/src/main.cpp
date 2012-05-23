/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <Launcher.h>

#include <windows.h>
#include <tchar.h>

#include <QtCore/QThread>
#include <QtCore/QCoreApplication>

#include <Log4Qt/LogManager>
#include <Log4Qt/Level>
#include <Log4Qt/FileAppender>
#include <Log4Qt/TTCCLayout>

#include <BugTrap/BugTrap.h>

using namespace Log4Qt;

void InitBugTrap()
{
   BT_SetAppName(_T("QGNA Application Launcher"));
   BT_SetSupportEMail(_T("support@gamenet.ru"));
   BT_SetFlags(BTF_DETAILEDMODE | BTF_ATTACHREPORT | BTF_SCREENCAPTURE);
   BT_SetSupportServer(_T("fs1.gamenet.ru"), 9999);

   BT_InstallSehFilter();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    InitBugTrap();

    //--- Start of prepare log4qt    
    TTCCLayout layout(TTCCLayout::ISO8601);
    layout.retain();

    FileAppender appender(&layout, a.applicationDirPath() + "/launcher.log", true);
    appender.retain();
    appender.activateOptions();
    
    LogManager::qtLogger()->addAppender(&appender);
#ifdef _DEBUG
    LogManager::setThreshold(Level::ALL_INT);
#else
    //LogManager::setThreshold(Level::WARN_INT);
    LogManager::setThreshold(Level::ALL_INT);
#endif
    LogManager::setHandleQtMessages(true);
    //--- End of prepare log4qt

    DEBUG_LOG << "Starting application"; 

    Launcher app;
    app.exec();

    int result = a.exec();
    
    DEBUG_LOG << "Shutdown application";
    LogManager::qtLogger()->removeAllAppenders(); 
    
    return result;
}