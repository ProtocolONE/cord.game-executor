/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_GAMEEXECUTORSERVICE_H
#define _GGS_GAMEEXECUTOR_GAMEEXECUTORSERVICE_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/ExecutorBase.h>
#include <GameExecutor/HookInterface.h>

#include <Core/Service>
#include <RestApi/RestApiManager>

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QMultiMap>

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

      void setRestApiManager(RestApi::RestApiManager *restApiManager);
      RestApi::RestApiManager *respApiManager();

      bool registerExecutor(ExecutorBase *executor);

      bool hasExecutor(const QString &scheme) const;

      bool addHook(const Core::Service &service, HookInterface* hook, int priority = 0);

      void clearHooks(const Core::Service &service);

      void execute(const Core::Service &service);

      bool isAnyGameStarted();

    signals:
      void canExecuteCompleted(const Core::Service &service);
      void preExecuteCompleted(const Core::Service &service);
      void started(const Core::Service &service);
      void finished(const Core::Service &service, GGS::GameExecutor::FinishState state);

    private slots:
      void privateFinished(const Core::Service &service, GGS::GameExecutor::FinishState state);
   
    private:
      RestApi::RestApiManager *_restApiManager;
      QHash<QString, ExecutorBase*> _executors;
      QHash<QString, QMultiMap<int, HookInterface*>> _hooks;
      QSet<QString> _startedServices;
      QMutex _lock;
    };
  }
}
#endif // _GGS_GAMEEXECUTOR_GAMEEXECUTORSERVICE_H
