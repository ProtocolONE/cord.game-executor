#include <GameExecutor/IPC/ClientServerPingPongTest.h>

ClientServerPingPongTest::ClientServerPingPongTest(QObject *parent)
  : QObject(parent)
{

  this->_client = new IPC::Client("testPipe", this);
  this->_server = new IPC::Server("testPipe", this);
 
  connect(this->_server, SIGNAL(messageReceived(int, QString)),
          this, SLOT(serverReceiveMessageFromClient(int, QString)));

  connect(this->_server, SIGNAL(clientConnected(int)), this, SLOT(serverNewClient(int)));

  connect(this->_client, SIGNAL(messageReceived(QString)),
    this, SLOT(clientReceiveMessageFromServer(QString)));

  connect(this->_client, SIGNAL(connected()), this, SLOT(clientConnectToServer()));
}

ClientServerPingPongTest::~ClientServerPingPongTest(void)
{
}

void ClientServerPingPongTest::timeoutTick()
{
  this->_loop.quit();
}

void ClientServerPingPongTest::testSendReceiveMessage()
{
  this->_messagesSendedFromClient.clear();
  this->_messagesSendedFromServer.clear();
  this->_messagesReceivedFromClient.clear();
  this->_messagesReceivedFromServer.clear();

  this->_server->start();
  this->_client->connectToServer();

  QTimer::singleShot(10000, this, SLOT(timeoutTick()));
  
  this->_loop.exec();
}

void ClientServerPingPongTest::clientConnectToServer()
{
  IPC::Client *client = qobject_cast<IPC::Client *>(QObject::sender());
  if (!client) {
    return; 
  }

  QString message = QString("clientHello %1").arg(qrand());
  this->_messagesSendedFromClient.append(message);
  client->sendMessage(message);
}

void ClientServerPingPongTest::serverNewClient(int id)
{
}

void ClientServerPingPongTest::clientReceiveMessageFromServer(QString message)
{
  IPC::Client *client = qobject_cast<IPC::Client *>(QObject::sender());
  if (!client) {
    return; 
  }

  this->_messagesReceivedFromServer.append(message);

  QString pongMessage = QString("clientPongMessage %1").arg(qrand());
  this->_messagesSendedFromClient.append(pongMessage);

  client->sendMessage(pongMessage);
}

void ClientServerPingPongTest::serverReceiveMessageFromClient(int id, QString message)
{
  this->_messagesReceivedFromClient.append(message);

  if (this->_messagesReceivedFromClient.count() > 4) {
    this->_loop.exit();
    return;
  }

  QString responseMessage = QString("serverPingMessage %1").arg(qrand());
  this->_messagesSendedFromServer.append(responseMessage);

  this->_server->sendMessage(id, responseMessage);
}

bool ClientServerPingPongTest::isClientAndServerReceiveAllMessages()
{
  foreach (const QString &i, this->_messagesSendedFromClient) {
    if (!this->_messagesReceivedFromClient.contains(i)) {
      return false;
    }
  }

  foreach (const QString &i, this->_messagesSendedFromServer) {
    if (!this->_messagesReceivedFromServer.contains(i)) {
      return false;
    }
  } 

  return true;
}


