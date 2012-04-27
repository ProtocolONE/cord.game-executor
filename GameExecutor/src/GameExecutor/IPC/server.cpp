#include <GameExecutor/IPC/server.h>
namespace GGS {
  namespace GameExecutor {
    namespace IPC {
      Server::Server(QObject *parent /*= 0*/) : QObject(parent), _clientIndex(0)
      {
          this->init();
      }

      Server::Server(const QString &name, QObject *parent) : QObject(parent), _clientIndex(0), _name(name)
      {
        Q_ASSERT(_name.length() != 0);
        this->init();
      }
      
      Server::~Server()
      {
      }

      void Server::init()
      {
        this->_server = new QLocalServer(this);
        connect(this->_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
      }

      void Server::setName(const QString &name)
      {
        this->_name = name;
      }

      bool Server::start()
      {
        return this->_server->listen(this->_name);
      }

      void Server::newConnection() 
      {
        QLocalSocket *clientConnection = this->_server->nextPendingConnection();

        connect(clientConnection, SIGNAL(disconnected()), this, SLOT(clientDisconected()));
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(clientConnection, SIGNAL(error(QLocalSocket::LocalSocketError)), 
                this, SLOT(clientError(QLocalSocket::LocalSocketError)));

        this->_clients.insert(++this->_clientIndex, clientConnection);
        emit this->clientConnected(this->_clientIndex);
      }

      void Server::clientDisconected()
      {
        QLocalSocket *client = qobject_cast<QLocalSocket *>(QObject::sender());
        if (!client) {
          qWarning() << __LINE__ << __FUNCTION__;
          return;
        }

        int id = this->getClientIdByLocalSocket(client);
        if (0 == id) {
          qDebug() << __LINE__ << __FUNCTION__;
        }
        
        this->_clients.remove(id);

        disconnect(client, SIGNAL(disconnected()), this, SLOT(clientDisconected()));
        disconnect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), 
          this, SLOT(clientError(QLocalSocket::LocalSocketError)));

        client->deleteLater();
      }

      void Server::clientError(QLocalSocket::LocalSocketError error)
      {
        QLocalSocket *client = qobject_cast<QLocalSocket *>(QObject::sender());
        if (!client) {
          qWarning() << __LINE__ << __FUNCTION__;
          return;
        }

        int id = this->getClientIdByLocalSocket(client);
        if (0 == id) {
          qWarning() << __LINE__ << __FUNCTION__;
        }

        emit this->error(id, PeerClosedError);
      }

      void Server::sendMessage(int id, const QString &message)
      {
        if (!this->_clients.contains(id)) {
          qWarning() << __LINE__ << __FUNCTION__ << id;
          emit this->error(id, PeerNotExistsError);
          return;
        }

        QLocalSocket *clientConnection = this->_clients[id];

        QByteArray block;
        QDataStream out(&block, QIODevice::ReadWrite);
        out.setVersion(QDataStream::Qt_4_7);
        out << message;
        out.device()->seek(0);
        clientConnection->write(block);
        clientConnection->flush();
      }

      void Server::readyRead()
      {
        QLocalSocket *reply = qobject_cast<QLocalSocket *>(QObject::sender());
        if (!reply) {
          qWarning() << __LINE__ << __FUNCTION__;
          return;
        }
        
        QString message;

        QDataStream in(reply);
        in >> message;
        
        int id = this->getClientIdByLocalSocket(reply);
        if (id == 0) {
          qCritical() << __LINE__ << __FUNCTION__;
        }
        
        emit this->messageReceived(id, message);
      }

      int Server::getClientIdByLocalSocket(const QLocalSocket *socket) const
      {
        Q_FOREACH(int id, this->_clients.keys()) {
          QLocalSocket *currentSocket = this->_clients.value(id);
          if (currentSocket == socket) {
            return id;
          }
        }

        return 0;
      }
    }
  }
}