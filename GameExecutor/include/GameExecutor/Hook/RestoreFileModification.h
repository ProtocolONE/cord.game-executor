#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QString>

namespace P1 {
  namespace GameExecutor {
    namespace Hook {

      /*!
        \class Хук запоминает время модификации файлов перед запуском игры и восстанавливает после завершения.
        \brief Restore file modification. 
        \author Ilya.Tkachenko
        \date 28.06.2012
      */
      class GAMEEXECUTOR_EXPORT RestoreFileModification : public HookInterface
      {
        Q_OBJECT
      public:
        explicit RestoreFileModification(QObject *parent = 0);
        virtual ~RestoreFileModification();

        static QString id();

        void PreExecute(Core::Service &service);
        void PostExecute(Core::Service &service, P1::GameExecutor::FinishState state);

      private:
        QHash<QString, QPair<quint32, quint32> > _modifiedTime;
      };

    }
  }
}
