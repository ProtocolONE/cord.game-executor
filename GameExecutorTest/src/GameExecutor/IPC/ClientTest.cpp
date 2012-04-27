#include <GameExecutor/IPC/ClientTest.h>

ClientTest::ClientTest(QObject *parent) : QObject(parent)
{
  this->_client = new IPC::Client("testPipe", this);
  this->_server = 0;
}

ClientTest::~ClientTest()
{

}

void ClientTest::connected()
{
  disconnect(this->_client, SIGNAL(connected()), this, SLOT(connected()));

  this->_connected = true;
}

void ClientTest::disconnected()
{
  disconnect(this->_client, SIGNAL(connected()), this, SLOT(connectedForDisconnectionTest()));
  disconnect(this->_client, SIGNAL(disconnected()), this, SLOT(disconnected()));

  this->_disconnected = true;
}

void ClientTest::testConnectionSlot()
{
  this->createServer();

  connect(this->_client, SIGNAL(connected()), this, SLOT(connected()));
  this->_client->connectToServer();
}

void ClientTest::testDisconnectionSlot()
{
  this->createServer();

  connect(this->_client, SIGNAL(connected()), this, SLOT(connectedForDisconnectionTest()), Qt::DirectConnection);
  connect(this->_client, SIGNAL(disconnected()), this, SLOT(disconnected()), Qt::DirectConnection);

  this->_client->connectToServer();
}

void ClientTest::connectedForDisconnectionTest()
{
  this->createServer();
}

void ClientTest::createServer()
{
  if (this->_server)
    delete this->_server;

  this->_server = new IPC::Server("testPipe", this);
  this->_server->start();
}

void ClientTest::error(QLocalSocket::LocalSocketError error)
{
  this->_error = true;
}

void ClientTest::testErrorSlot()
{
  connect(this->_client, SIGNAL(error(QLocalSocket::LocalSocketError)), 
          this, SLOT(error(QLocalSocket::LocalSocketError)), Qt::DirectConnection);
  
  this->_client->sendMessage("errorMessage");
}