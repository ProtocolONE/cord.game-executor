#include <GameExecutor/Hook/DownloadCustomFile.h>
#include <Settings/Settings>

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

      void DownloadCustomFile::CanExecute(GGS::Core::Service &service)
      {
        QUrl url = service.url();
        if (!url.hasQueryItem("downloadCustomFile")) {
          emit this->canExecuteCompleted(service, GGS::GameExecutor::Success);
          return;
        }
        
        this->_args = url.queryItemValue("downloadCustomFile").split(',');

        if ((this->_args.count() % 3)) {
          emit this->canExecuteCompleted(service, GGS::GameExecutor::CanExecutionHookBreak);
          return;
        }

        this->_baseFilePath = service.installPath();
        this->_area = service.areaString();

        this->_service = service;
        this->downloadNextFile();
      }

      void DownloadCustomFile::downloadNextFile()
      {
        if (this->_args.isEmpty()) {
           emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::Success);
           return;
        }

        QString relativeFilePath = this->_args.takeFirst();

        if (this->_file.isOpen())
          this->_file.close();

		this->_downloadFilePath = QString("%1/%2/%3").arg(this->_baseFilePath, this->_area, relativeFilePath);

        this->_file.setFileName(this->_downloadFilePath);

        QFileInfo fileInfo(this->_file);

        if (!fileInfo.absoluteDir().mkpath(fileInfo.absoluteDir().absolutePath())) {
          emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
          return;
        }

        this->_downloadUrl = QUrl(QString("%1/%2/").arg(this->_args.takeFirst(), this->_area));

        int mode = this->_args.takeFirst().toInt();
        if (mode == 0 && fileInfo.exists()) {
          this->downloadNextFile();
          return;
        }

		this->_mode = mode;

		if (mode == 2 && QFile::exists(this->_downloadFilePath)) {
			this->_url = this->_downloadUrl.resolved(QUrl(relativeFilePath));

			QNetworkRequest request(this->_url);
			QString oldLastModifed = this->loadLastModifiedDate(this->_url.toString());
			request.setRawHeader("If-Modified-Since", oldLastModifed.toAscii());

			QNetworkReply *reply = this->_manager.head(request);
			connect(reply, SIGNAL(finished()), this, SLOT(slotReplyDownloadFinished()));
			connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));

			return;
		}

		this->_url = this->_downloadUrl.resolved(QUrl(relativeFilePath));
		this->_info = QHostInfo::fromName(this->_url.host());
		this->_infoIndex = this->_info.addresses().size();

		if (!this->_file.open(QIODevice::ReadWrite)) {
			emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
			return;
		}

		this->downloadFile();
	  }

	  void DownloadCustomFile::slotError(QNetworkReply::NetworkError error) 
	  {
          emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
	  }

	  void DownloadCustomFile::slotReplyDownloadFinished() 
	  {
		  QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
		  if (!reply)
			  return;

		  reply->deleteLater();

		  int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		  // Http codes defined by rfc: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
		  // 200 OK
		  // 304 Not Modified
		  if (httpCode != 304 && httpCode != 200) {
			  CRITICAL_LOG << "Http error" << httpCode;
			  emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
			  return;
		  } 

		  this->_lastModified = QString::fromAscii(reply->rawHeader(QByteArray("Last-Modified")));

		  if (httpCode == 304 &&
			  QFile::exists(this->_downloadFilePath)) {
			  emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::Success);
			  return;
		  }

		  if (!this->_file.open(QIODevice::ReadWrite)) {
			  emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
			  return;
		  }
		  		
		  this->_info = QHostInfo::fromName(this->_url.host());
		  this->_infoIndex = this->_info.addresses().size();

		  this->downloadFile();
	  }

	  QString DownloadCustomFile::loadLastModifiedDate(const QString& url) const
	  {
		  Settings::Settings settings; 
		  settings.beginGroup("GameExecutor");
		  settings.beginGroup("CheckFileUpdate");
		  settings.beginGroup(this->_service.id());
		  settings.beginGroup(url);
		  return settings.value("LastModified", "").toString();
	  }

	  void DownloadCustomFile::saveLoadLastModifiedDate(const QString& url, const QString& value) {
		  Settings::Settings settings; 
		  settings.beginGroup("GameExecutor");
		  settings.beginGroup("CheckFileUpdate");
		  settings.beginGroup(this->_service.id());
		  settings.beginGroup(url);
		  settings.setValue("LastModified", value);
	  }

      void DownloadCustomFile::downloadFile()
      {
        if (--this->_infoIndex < 0) {
          emit this->canExecuteCompleted(this->_service, GGS::GameExecutor::CanExecutionHookBreak);
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

		if (this->_mode == 2)
		  this->saveLoadLastModifiedDate(this->_url.toString(), this->_lastModified);

        this->downloadNextFile();
      }
    }
  }
}