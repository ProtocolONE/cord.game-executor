/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2014, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <RestApi/CommandBase.h>

class QTimer;

namespace GGS {
  namespace RestApi {
    namespace Commands {
      namespace User {
        class SetUserActivity;
      }
    }
  }

  namespace GameExecutor {
    namespace Hook {

      class GAMEEXECUTOR_EXPORT SendPlayingInfo : public HookInterface
      {
      public:
        explicit SendPlayingInfo(QObject *parent = 0);
        virtual ~SendPlayingInfo();
        
        static QString id();

        virtual void PreExecute(Core::Service &service) override;
        virtual void PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state) override;

      private:
        GGS::RestApi::Commands::User::SetUserActivity *_playingCmd;
        QTimer *_timer;

        void playingResult(GGS::RestApi::CommandBase::CommandResults result);
        void sendPlaying();
      };

    }
  }
}