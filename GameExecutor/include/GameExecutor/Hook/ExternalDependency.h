#pragma once

#include <Core/Service.h>
#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QStringList>
#include <QObject>

namespace P1 {
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

        virtual void PostExecute(Core::Service &service, P1::GameExecutor::FinishState state) override;
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

