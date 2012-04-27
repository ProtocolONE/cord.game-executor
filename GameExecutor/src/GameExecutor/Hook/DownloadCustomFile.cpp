#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <QtCore/QEventLoop>
#include <QtCore/QScopedPointer>

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QDataStream>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      DownloadCustomFile::DownloadCustomFile(QObject *parent) : QObject(parent)
      {
      }

      DownloadCustomFile::~DownloadCustomFile()
      {
      }

      bool DownloadCustomFile::CanExecute(const Core::Service &service)
      {
        QUrl url = service.url();
        if (!url.hasQueryItem("downloadCustomFile"))
          return true;
               
        QString relativeFilePath = url.queryItemValue("downloadCustomFile");

        QFile file(service.extractionPath() + "/" + relativeFilePath);

        if (!url.hasQueryItem("downloadCustomFileOverride") && file.exists()) {
          return true;
        };

        QUrl baseUrl = service.torrentUrlWithArea();
        QUrl fileUrl = baseUrl.resolved(QUrl(relativeFilePath));
      
        bool result = this->DownloadFile(fileUrl, file);
        if (!result) {
          //UNDONE Нет механизма показать ошибку во всплывающем окне
          //someManager::ShowErrorAboutGameFileDownloading
        }

        return result;
      }

      bool DownloadCustomFile::DownloadFile(QUrl &fileUrl, QFile &file)
      {
        QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager(this));

        QHostInfo info = QHostInfo::fromName(fileUrl.host());

        Q_FOREACH(QHostAddress address, info.addresses()) {
          fileUrl.setHost(address.toString());

          QNetworkReply *reply = manager->get(QNetworkRequest(fileUrl));

          QEventLoop el;
          connect(reply, SIGNAL(finished()), &el, SLOT(quit())); 
          el.exec();

          if (QNetworkReply::NoError != reply->error()) {
            reply->deleteLater();
            continue;
          }

          if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            reply->deleteLater();
            return false;
          }

          file.write(reply->readAll());
          file.flush();
          file.close();

          reply->deleteLater();
          return true;
        }

        return false;
      }

      void DownloadCustomFile::PostExecute(const Core::Service &service, GGS::GameExecutor::FinishState state)
      {
      }

      bool DownloadCustomFile::PreExecute(const Core::Service &service)
      {
        return true;
      }
    }
  }
}