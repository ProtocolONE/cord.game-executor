#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      DownloadCustomFile::DownloadCustomFile(QObject *parent) : HookInterface(parent)
      {
      }

      DownloadCustomFile::~DownloadCustomFile()
      {
      }

      void DownloadCustomFile::CanExecute(const Core::Service &service)
      {
        QUrl url = service.url();
        if (!url.hasQueryItem("downloadCustomFile")) {
          emit this->canExecuteCompleted(true);
          return;
        }

        QString relativeFilePath = url.queryItemValue("downloadCustomFile");

        QString path = QString("%1/%2/%3").arg(service.installPath(), service.areaString(), relativeFilePath);
        
        this->_file.setFileName(path);
        QFileInfo fileInfo(this->_file);

        if (!fileInfo.absoluteDir().mkpath(fileInfo.absoluteDir().absolutePath())) {
          emit this->canExecuteCompleted(false);
          return;
        }

        if (!url.hasQueryItem("downloadCustomFileOverride") && fileInfo.exists()) {
          emit this->canExecuteCompleted(false);
          return;
        };

        if (url.hasQueryItem("downloadCustomFileUrl")) {
          QUrl baseUrl(url.queryItemValue("downloadCustomFileUrl"));
          baseUrl = baseUrl.resolved("./" + service.areaString() + "/");

          this->_url = baseUrl.resolved(QUrl(relativeFilePath));
          DEBUG_LOG << "custom download" << this->_url.toString();
        } else {
          QUrl baseUrl = service.torrentUrlWithArea();
          this->_url = baseUrl.resolved(QUrl(relativeFilePath));
        }

        this->_info = QHostInfo::fromName(this->_url.host());
        this->_infoIndex = this->_info.addresses().size();

        this->DownloadFile();
      }

      void DownloadCustomFile::DownloadFile()
      {
        if (--this->_infoIndex < 0) {
          emit this->canExecuteCompleted(false);
          return;
        }

        this->_url.setHost(this->_info.addresses().at(this->_infoIndex).toString());

        QNetworkReply *reply = this->_manager.get(QNetworkRequest(this->_url));
        connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
      }

      void DownloadCustomFile::requestFinished()
      {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
        reply->deleteLater();

        if (QNetworkReply::NoError != reply->error()) {
          this->DownloadFile();
          return;
        }
        
        if (!this->_file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
          emit this->canExecuteCompleted(false);
          return ;
        }

        this->_file.write(reply->readAll());
        this->_file.flush();
        this->_file.close();

        emit this->canExecuteCompleted(true);
      }
    }
  }
}