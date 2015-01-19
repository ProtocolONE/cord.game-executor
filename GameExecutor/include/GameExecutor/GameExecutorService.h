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
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/HookInterface.h>
#include <GameExecutor/Extension.h>

#include <Core/Service.h>
#include <RestApi/RestApiManager.h>

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QMultiMap>

#include <functional>

namespace GGS {
  namespace GameExecutor {

    /*!
      \class GameExecutorService
      \brief Управляет запуском игр.
      \ingroup GameExecutor

      Важно, что последовательность 

      \sa Core::Service, ExecutorBase, HookInterface
    */
    class GAMEEXECUTOR_EXPORT GameExecutorService : public QObject
    {
      Q_OBJECT
    public:
      explicit GameExecutorService(QObject *parent = 0);
      ~GameExecutorService();

      bool registerExecutor(ExecutorBase *executor);
      bool hasExecutor(const QString &scheme) const;

      virtual bool addHook(const GGS::Core::Service &service, HookInterface* hook, int priority = 0);
      void clearHooks(const GGS::Core::Service &service);

      __declspec(deprecated("This is deprecated function at 08.09.2014. Use executeEx instead of execute."))
        void execute(const GGS::Core::Service &service);

      __declspec(deprecated("This is deprecated function at 08.09.2014. Use executeEx instead of execute.")) 
        void execute(const GGS::Core::Service &service, const GGS::RestApi::GameNetCredential &credential);

      /**
       * \fn  void GameExecutorService::executeEx( const GGS::Core::Service &service,
       *      const GGS::RestApi::GameNetCredential &credential,
       *      const GGS::RestApi::GameNetCredential &secondCredential = GGS::RestApi::GameNetCredential());
       *
       * \brief Executes the ex operation.
       *
       * \author  Ilya Tkachenko
       * \date  08.09.2014
       *
       * \param service           Запускаемый сервис.
       * \param credential        Авторизация основной учетной записи.
       * \param secondCredential  Авторизация дополнительной учетной записи. Если она не пустая, то
       *                          игра будет запускаться от этой учетной записи, но с проверкой доступа по
       *                          основной.
       */

      void executeEx(
        const GGS::Core::Service &service, 
        const GGS::RestApi::GameNetCredential &credential,
        const GGS::RestApi::GameNetCredential &secondCredential = GGS::RestApi::GameNetCredential());

      void terminateAll();

      bool isAnyGameStarted();
      bool isGameStarted(const QString& serviceId);

      void shutdown();

      void setExtension(ExtensionTypes::Values type, void* extension);
      void* extension(ExtensionTypes::Values type);

    signals:
      void canExecuteCompleted(const GGS::Core::Service &service);
      void preExecuteCompleted(const GGS::Core::Service &service);
      void started(const GGS::Core::Service &service);
      void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);
      void stopExecution();

    private slots:
      void privateFinished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);
   
    private:
      QHash<QString, ExecutorBase*> _executors;
      QHash<QString, QMultiMap<int, HookInterface*>> _hooks;
      QSet<QString> _startedServices;
      QMutex _lock;

      QHash<int, void*> _extensionsMap;
    };
  }
}
