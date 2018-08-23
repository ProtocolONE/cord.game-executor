#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <RestApi/CommandBase.h>

class QTimer;

namespace P1 {
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
        virtual void PostExecute(Core::Service &service, P1::GameExecutor::FinishState state) override;

      private:
        P1::RestApi::Commands::User::SetUserActivity *_playingCmd;
        QTimer *_timer;

        void playingResult(P1::RestApi::CommandBase::CommandResults result);
        void sendPlaying();
      };

    }
  }
}
