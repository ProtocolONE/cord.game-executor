#include <GameExecutor/Hook/BannerDownload.h>

#include <RestApi/Commands/Games/GetCAInGameBanner.h>

#include <QtCore/QFile>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

using GGS::RestApi::Commands::Games::GetCAInGameBanner;

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      BannerDownload::BannerDownload(QObject *parent /*= 0*/)
      {

      }

      BannerDownload::~BannerDownload()
      {

      }

      void BannerDownload::PreExecute(Core::Service &service)
      {
        if (service.id() != "300009010000000000") {
          this->preExecuteCompleted(service, GGS::GameExecutor::FinishState::Success);
          return;
        }

        this->_service = service;

        GetCAInGameBanner *cmd = new GetCAInGameBanner();
        SIGNAL_CONNECT_CHECK(connect(cmd, SIGNAL(result(GGS::RestApi::CommandBase::CommandResults)),
          this, SLOT(userMainInfoResult(GGS::RestApi::CommandBase::CommandResults))));

        cmd->execute();
      }

      void BannerDownload::userMainInfoResult(GGS::RestApi::CommandBase::CommandResults code)
      {
        GetCAInGameBanner *cmd = qobject_cast<GetCAInGameBanner *>(QObject::sender());

        if (!cmd) {
          this->preExecuteCompleted(this->_service, GGS::GameExecutor::FinishState::Success);
          return;
        }

        cmd->deleteLater();

        if (code != GGS::RestApi::CommandBase::NoError) {
          this->preExecuteCompleted(this->_service, GGS::GameExecutor::FinishState::Success);
          return;
        }

        QString imageUrl = cmd->imageUrl();

        QNetworkReply* reply = this->_manager.get(QNetworkRequest(imageUrl));
        SIGNAL_CONNECT_CHECK(connect(reply, SIGNAL(finished()), this, SLOT(requestFinished())));
      }

      void BannerDownload::requestFinished()
      {
        QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
        reply->deleteLater();

        if (QNetworkReply::NoError != reply->error()) {
          this->preExecuteCompleted(this->_service, GGS::GameExecutor::FinishState::Success);
          return;
        }

        QString path = QString("%1/%2/bannerca.jpg").arg(this->_service.installPath(), this->_service.areaString());
        QFile file(path);

        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
          this->preExecuteCompleted(this->_service, GGS::GameExecutor::FinishState::Success);
          return ;
        }

        file.write(reply->readAll());
        file.close();

        this->preExecuteCompleted(this->_service, GGS::GameExecutor::FinishState::Success);
      }

    }
  }
}