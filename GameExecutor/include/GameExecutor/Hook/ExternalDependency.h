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
#include <Core/Service>
#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QStringList>
#include <QObject>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      /*!
        \class RestoreResolution
      
        \brief Сохраняет и восстанавливает разрешение экрана после запуска игры. 
      */
      class GAMEEXECUTOR_EXPORT ExternalDependency : public HookInterface
      {
        Q_OBJECT
      public:
        explicit ExternalDependency(QObject *parent = 0);
        virtual ~ExternalDependency();

        static QString id();

        virtual void PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state) override;
        void setExternalDepencyList(const QString& dependencyList);

      private:
        void install(Core::Service &service);

        QStringList _fileNames;
        bool _isInstalled;

      signals:
        void externalDependencyInstalled(QString);
      };
    }
  }
}

