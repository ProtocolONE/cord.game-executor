/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_EXECUTORBASE_H
#define _GGS_GAMEEXECUTOR_EXECUTORBASE_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/Enum.h>

#include <Core/Service>
#include <QtCore/QObject>

namespace GGS {
  namespace GameExecutor {
    class GameExecutorService;

    class GAMEEXECUTOR_EXPORT ExecutorBase : public QObject
    {
      Q_OBJECT
    public:
      explicit ExecutorBase(QObject *parent = 0);
      ExecutorBase(const QString &scheme, QObject *parent = 0);
      virtual ~ExecutorBase();

      virtual void execute(const Core::Service &service, GameExecutorService *executorService) = 0;
      const QString &scheme() const;
    
    signals:
      void started(const GGS::Core::Service &service);
      void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);
        
    protected:
      QString _scheme;
    };
  }
}

#endif //_GGS_GAMEEXECUTOR_EXECUTORBASE_H

