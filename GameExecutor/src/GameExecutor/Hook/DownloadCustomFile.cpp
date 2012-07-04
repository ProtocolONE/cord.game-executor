#include <GameExecutor/Hook/DownloadCustomFile.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtXml/QXmlSimpleReader>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      DownloadCustomFile::DownloadCustomFile(QObject *parent) : HookInterface(parent)
      {
      }

      DownloadCustomFile::~DownloadCustomFile()
      {
      }

      void DownloadCustomFile::CanExecute(Core::Service &service)
      {
        QUrl url = service.url();
        if (!url.hasQueryItem("downloadCustomFile")) {
          emit this->canExecuteCompleted(GGS::GameExecutor::Success);
          return;
        }
        
        this->_args = url.queryItemValue("downloadCustomFile").split(',');

        if ((this->_args.count() % 2)) {
          emit this->canExecuteCompleted(GGS::GameExecutor::CanExecutionHookBreak);
          return;
        }

        this->_baseFilePath = service.installPath();
        this->_downloadUrl = service.torrentUrlWithArea();
        this->_area = service.areaString();

        this->downloadNextFile();
      }

      void DownloadCustomFile::downloadNextFile()
      {
        if (this->_args.isEmpty()) {
           emit this->canExecuteCompleted(GGS::GameExecutor::Success);
           return;
        }

        QString relativeFilePath = this->_args.takeFirst();

        this->_file.setFileName(
          QString("%1/%2/%3").arg(this->_baseFilePath, this->_area, relativeFilePath));

        QFileInfo fileInfo(this->_file);

        if (!fileInfo.absoluteDir().mkpath(fileInfo.absoluteDir().absolutePath())) {
          emit this->canExecuteCompleted(GGS::GameExecutor::CanExecutionHookBreak);
          return;
        }
                
        int mode = this->_args.takeFirst().toInt();
        if (mode == 0 && fileInfo.exists()) {
          this->downloadNextFile();
          return;
        }

        this->_url = this->_downloadUrl.resolved(QUrl(relativeFilePath));
        this->_info = QHostInfo::fromName(this->_url.host());
        this->_infoIndex = this->_info.addresses().size();
        
        if (!this->_file.open(QIODevice::ReadWrite)) {
          emit this->canExecuteCompleted(GGS::GameExecutor::CanExecutionHookBreak);
          return ;
        }

        this->downloadFile();
      }

      void DownloadCustomFile::downloadFile()
      {
        if (--this->_infoIndex < 0) {
          emit this->canExecuteCompleted(GGS::GameExecutor::CanExecutionHookBreak);
          return;
        }

        this->_url.setHost(this->_info.addresses().at(this->_infoIndex).toString());

        QNetworkReply* reply = this->_manager.get(QNetworkRequest(this->_url));
        connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
      }

      void DownloadCustomFile::requestFinished()
      {
        QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
        reply->deleteLater();

        if (QNetworkReply::NoError != reply->error()) {
          this->downloadFile();
          return;
        }
        
        this->_file.resize(0);
        this->_file.write(reply->readAll());
        this->_file.flush();
        this->_file.close();

        this->downloadNextFile();
      }
    }
  }
}