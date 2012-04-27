/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_HOOK_DOWNLOADCUSTOMFILE_H
#define _GGS_GAMEEXECUTOR_HOOK_DOWNLOADCUSTOMFILE_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QObject>
#include <QtCore/QUrl>
#include <QtCore/QFile>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      /*!
        \class DownloadCustomFile
      
        \brief Позволяет загрузить произвольный файл перед запуском игры. Это нужно например для игры BS и MW2 где
        подобным образом загружаюьтся файлы конфигурации игры.
      */
      class GAMEEXECUTOR_EXPORT DownloadCustomFile : public QObject, public HookInterface
      {
        Q_OBJECT
      public:
        DownloadCustomFile(QObject *parent = 0);
        ~DownloadCustomFile();

        virtual bool CanExecute(const Core::Service &service);

        virtual void PostExecute(const Core::Service &service, GGS::GameExecutor::FinishState state);

        virtual bool PreExecute(const Core::Service &service);

      private:
        bool DownloadFile(QUrl &fileUrl, QFile &file);
      };
    }
  }
}
#endif // _GGS_GAMEEXECUTOR_HOOK_DOWNLOADCUSTOMFILE_H
