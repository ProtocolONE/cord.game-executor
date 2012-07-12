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

      bool registerExecutor(ExecutorBase *executor);

      bool hasExecutor(const QString &scheme) const;

      bool addHook(const GGS::Core::Service &service, HookInterface* hook, int priority = 0);

      void clearHooks(const GGS::Core::Service &service);

      void execute(const GGS::Core::Service &service);

      bool isAnyGameStarted();

    signals:
      void canExecuteCompleted(const GGS::Core::Service &service);
      void preExecuteCompleted(const GGS::Core::Service &service);
      void started(const GGS::Core::Service &service);
      void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);

    private slots:
      void privateFinished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);
   
    private:
      QHash<QString, ExecutorBase*> _executors;
      QHash<QString, QMultiMap<int, HookInterface*>> _hooks;
      QSet<QString> _startedServices;
      QMutex _lock;
    };
  }
}
#endif // _GGS_GAMEEXECUTOR_GAMEEXECUTORSERVICE_H
