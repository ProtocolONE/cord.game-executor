/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_IPC_SERVER_H
#define _GGS_GAMEEXECUTOR_IPC_SERVER_H

#include <GameExecutor/gameexecutor_global.h>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace GameExecutor {
    namespace IPC {

      /*!
        \class Server
        \brief Half-Duplex pipe server.
      */
      class GAMEEXECUTOR_EXPORT Server : public QObject
      {
        Q_OBJECT
      public:
        enum ServerError { PeerClosedError = 0, PeerNotExistsError };

        explicit Server(QObject *parent = 0);
        Server(const QString &name, QObject *parent = 0);
        ~Server();
        
        void setName(const QString &name);;
        void sendMessage(int id, const QString &message);
        bool start();
      signals:
        void messageReceived(int id, QString message);
        void clientConnected(int id);
        void error(int id, GGS::GameExecutor::IPC::Server::ServerError error);

      public slots:
        void newConnection();
        void clientDisconected();
        void readyRead();
        void clientError(QLocalSocket::LocalSocketError error);

      private:
        void init();
        int getClientIdByLocalSocket(const QLocalSocket *socket) const;
        
        int _clientIndex;
        
        QString _name;
        QLocalServer *_server;
        QMap<int, QLocalSocket *> _clients;
      };
    }
  }
}

#endif // _GGS_GAMEEXECUTOR_IPC_SERVER_H
