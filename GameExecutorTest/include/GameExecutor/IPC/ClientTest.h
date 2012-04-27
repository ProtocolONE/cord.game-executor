/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _TEST_GAMEEXECUTOR_IPC_CLIENTTEST_H
#define _TEST_GAMEEXECUTOR_IPC_CLIENTTEST_H

#include <GameExecutor/IPC/Server.h>
#include <GameExecutor/IPC/Client.h>

#include <QtGlobal>
#include <QtCore/QObject>

using namespace GGS::GameExecutor;

class ClientTest : public QObject
{
  Q_OBJECT

public:
  ClientTest(QObject *parent = 0);
  ~ClientTest();

  void testConnectionSlot();
  void testDisconnectionSlot();
  void testErrorSlot();

  bool isConnected() const { return this->_connected; };
  bool isDisconnected() const { return this->_disconnected; };
  bool isError() const { return this->_error; };

public slots:
  void connected();

  void disconnected();
  void connectedForDisconnectionTest();

  void error(QLocalSocket::LocalSocketError error);

private:
  void createServer();

  IPC::Server *_server;
  IPC::Client *_client;

  bool _connected;
  bool _disconnected;
  bool _error;
};

#endif // _TEST_GAMEEXECUTOR_IPC_CLIENTTEST_H
