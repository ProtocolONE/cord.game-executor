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

#include <GameExecutor/IPC/ClientTest.h>
#include <GameExecutor/IPC/ClientServerPingPongTest.h>

//TODO Мой первый тест, поэтому он какой-то корявый. Переделать?
TEST(ServerClientIPC, ServerMultiStart) 
{
  IPC::Server server("testPipe");
  ASSERT_TRUE(server.start());
  ASSERT_FALSE(server.start());
}

TEST(ServerClientIPC, SendReceiveMessage) 
{
  ClientServerPingPongTest test;
  test.testSendReceiveMessage();

  ASSERT_TRUE(test.isClientAndServerReceiveAllMessages());
}

TEST(ServerClientIPC, ClientConnectionSlot) 
{
  ClientTest test;
  test.testConnectionSlot();

  ASSERT_TRUE(test.isConnected());
}

TEST(ServerClientIPC, ClientDisconnectionSlot) 
{
  ClientTest test;
  test.testDisconnectionSlot();

  ASSERT_TRUE(test.isDisconnected());
}

TEST(ServerClientIPC, DISABLED_ClientErrorSlot) 
{
  ClientTest test;
  test.testErrorSlot();

  ASSERT_TRUE(test.isError());
}
