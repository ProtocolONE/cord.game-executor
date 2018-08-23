#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkReply>

namespace P1 {
  namespace GameExecutor {
    namespace Hook {

      /*!
      \class DownloadCustomFile

      \brief Позволяет загрузить произвольный файл перед запуском игры. Это нужно например для игры BS и MW2 где
      подобным образом загружаюьтся файлы конфигурации игры.

      \code
      QUrl url;
      url.addQueryItem("downloadCustomFile", "./launcher/serverinfo_back.xml,1,./config/lastlogin.xml,0");
      \code
      */
      class GAMEEXECUTOR_EXPORT DownloadCustomFile : public HookInterface
      {
        Q_OBJECT
      public:
        explicit DownloadCustomFile(QObject *parent = 0);
        virtual ~DownloadCustomFile();

        static QString id();

        virtual void CanExecute(Core::Service &service);

      public slots:
        void requestFinished();

      private slots:
        void slotError(QNetworkReply::NetworkError error);
        void slotReplyDownloadFinished();

      private:
        void downloadNextFile();
        void downloadFile();
        QString loadLastModifiedDate(const QString& url) const;
        void saveLoadLastModifiedDate(const QString& url, const QString& value);

        int _mode;
        QStringList _args;
        QUrl _url;
        QUrl _downloadUrl;
        QString _lastModified;
        QString _baseFilePath;
        QString _area;
        QString _downloadFilePath;
        QHostInfo _info;
        qint32 _infoIndex;
        QFile _file;
        QNetworkAccessManager _manager;

        P1::Core::Service _service;
      };
    }
  }
}
