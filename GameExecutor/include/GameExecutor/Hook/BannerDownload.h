/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <RestApi/CommandBase.h>

#include <QtNetwork/QNetworkAccessManager>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      class GAMEEXECUTOR_EXPORT BannerDownload : public HookInterface
      {
        Q_OBJECT
      public:
        BannerDownload(QObject *parent = 0);
        ~BannerDownload();

        virtual void PreExecute(Core::Service &service);

      private slots:
        void userMainInfoResult(GGS::RestApi::CommandBase::CommandResults code);
        void requestFinished();

      private:
        GGS::Core::Service _service;
        QNetworkAccessManager _manager;
      };

    }
  }
}