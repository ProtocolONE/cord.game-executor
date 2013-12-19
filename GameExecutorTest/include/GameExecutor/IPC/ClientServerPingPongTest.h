/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/IPC/Server.h>
#include <GameExecutor/IPC/Client.h>

#include <QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QList>

using namespace GGS::GameExecutor;

class ClientServerPingPongTest : public QObject
{
  Q_OBJECT
public:
  ClientServerPingPongTest(QObject *parent = 0);
  ~ClientServerPingPongTest(void);
  
  void testSendReceiveMessage();
  bool isClientAndServerReceiveAllMessages();

public slots:
  void timeoutTick();

  void serverReceiveMessageFromClient(int id, QString message);
  void clientReceiveMessageFromServer(QString message);

  void clientConnectToServer();
  void serverNewClient(int id);
  
private:
  IPC::Server *_server;
  IPC::Client *_client;

  QList<QString> _messagesSendedFromClient;
  QList<QString> _messagesReceivedFromServer;
  QList<QString> _messagesSendedFromServer;
  QList<QString> _messagesReceivedFromClient;

  QEventLoop _loop;
};


