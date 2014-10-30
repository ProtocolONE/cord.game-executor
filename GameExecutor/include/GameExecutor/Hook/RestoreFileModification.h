/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXCUTOR_HOOK_RESTOREFILEMODIFICATION_H_
#define _GGS_GAMEEXCUTOR_HOOK_RESTOREFILEMODIFICATION_H_

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QString>

namespace GGS {
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
        void PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state);

      private:
        QHash<QString, QPair<quint32, quint32> > _modifiedTime;
      };

    }
  }
}
#endif // _GGS_GAMEEXCUTOR_HOOK_RESTOREFILEMODIFICATION_H_