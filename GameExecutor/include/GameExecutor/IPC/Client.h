/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_IPC_CLIENT_H
#define _GGS_GAMEEXECUTOR_IPC_CLIENT_H

#include <GameExecutor/gameexecutor_global.h>

#include <QtCore/QObject>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace GameExecutor {
    namespace IPC {
      /*!
        \class Client
        \brief Half-Duplex pipe client. 
      */
      class GAMEEXECUTOR_EXPORT Client : public QObject
      {
        Q_OBJECT
      public:
        explicit Client(QObject *parent = 0);
        Client(const QString &name, QObject *parent = 0);
        ~Client();

        void setName(const QString &name);

        void connectToServer();
        void sendMessage(const QString &message);
      
      signals:
        void messageReceived(QString message);
        void connected();
        void disconnected();
        void error(QLocalSocket::LocalSocketError error);
         
      private slots:
        void readReady();
      
      private:
        void init();

        QString _name;
        QLocalSocket *_socket;
      };
    }
  }
}

#endif // _GGS_GAMEEXECUTOR_IPC_CLIENT_H
