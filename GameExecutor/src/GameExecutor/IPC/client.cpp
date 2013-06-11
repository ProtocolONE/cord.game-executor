#include <GameExecutor/IPC/Client.h>
#include <QtCore/QDataStream>

namespace GGS {
  namespace GameExecutor {
    namespace IPC {
      Client::Client(QObject *parent /*= 0*/) : QObject(parent)
      {
        this->init();
      }

      Client::Client(const QString &name, QObject *parent /*= 0*/) : QObject(parent), _name(name)
      {
        Q_ASSERT(_name.length());
        this->init();
      }

      void Client::init()
      {
        this->_socket = new QLocalSocket(this);

        connect(this->_socket, SIGNAL(connected()), this, SIGNAL(connected()));
        connect(this->_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(this->_socket, SIGNAL(readyRead()), this, SLOT(readReady()));
        connect(this->_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
                this, SIGNAL(error(QLocalSocket::LocalSocketError)));
      }

      Client::~Client()
      {
        this->_socket->abort();
      }

      void Client::setName(const QString &name)
      {
        this->_name = name;
      }

      void Client::connectToServer()
      {
        this->_socket->abort();
        this->_socket->connectToServer(this->_name, QIODevice::ReadWrite);
      }

      void Client::sendMessage(const QString &message) 
      {
        QByteArray block;
        QDataStream out(&block, QIODevice::ReadWrite);
        out.setVersion(QDataStream::Qt_4_7);
        out << message;
        out.device()->seek(0);
        this->_socket->write(block);
        this->_socket->flush();
      }

      void Client::readReady() {
        QLocalSocket *reply = qobject_cast<QLocalSocket *>(QObject::sender());
        if (!reply) {
          WARNING_LOG << "wrong sender" << QObject::sender()->metaObject()->className();
          return;
        }

        QString message;

        QDataStream in(reply);
        in >> message;

        emit this->messageReceived(message);
      }
    }
  }
}

