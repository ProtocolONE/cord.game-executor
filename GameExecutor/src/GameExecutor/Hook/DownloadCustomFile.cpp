#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <QtCore/QEventLoop>
#include <QtCore/QScopedPointer>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

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
        DEBUG_LOG << "for" << service.id();

        QUrl url = service.url();
        if (!url.hasQueryItem("downloadCustomFile"))
          return true;
               
        QString relativeFilePath = url.queryItemValue("downloadCustomFile");

        QString path = QString("%1/%2/%3").arg(service.installPath(), service.areaString(), relativeFilePath);
        QFile file(path);
        QFileInfo fileInfo(file);

        if (!fileInfo.absoluteDir().mkpath(fileInfo.absoluteDir().absolutePath())) {
          return false;
        }

        if (!url.hasQueryItem("downloadCustomFileOverride") && file.exists()) {
          return true;
        };

        bool result;
        if (url.hasQueryItem("downloadCustomFileUrl")) {
          QUrl baseUrl(url.queryItemValue("downloadCustomFileUrl"));
          
          switch(service.area()){
          case GGS::Core::Service::Pts:
            baseUrl = baseUrl.resolved(QUrl("./pts/"));
            break;
          case GGS::Core::Service::Tst:
            baseUrl = baseUrl.resolved(QUrl("./tst/"));
            break;
          case GGS::Core::Service::Live:
            baseUrl = baseUrl.resolved(QUrl("./live/"));
            break;
          default:
            baseUrl = baseUrl.resolved(QUrl("./live/"));
            break;
          };

          QUrl fileUrl = baseUrl.resolved(QUrl(relativeFilePath));
          DEBUG_LOG << "custom download" << fileUrl.toString();
          result = this->DownloadFile(fileUrl, file);
        } else {
          QUrl baseUrl = service.torrentUrlWithArea();
          QUrl fileUrl = baseUrl.resolved(QUrl(relativeFilePath));

          result = this->DownloadFile(fileUrl, file);
        }

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