/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <gtest/gtest.h>
#include <QtWidgets/QApplication>

#include <GameExecutor/Enum.h>
#include <GameExecutor/IPC/ClientTest.h>
#include <GameExecutor/IPC/ClientServerPingPongTest.h>

#include <Core/Service>
#include <QMetaType>


#ifdef VLD_CHECK_ENABLED
#include <vld.h>
#pragma comment(lib, "vld.lib")
#endif

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  int r = RUN_ALL_TESTS();
  return r;
}