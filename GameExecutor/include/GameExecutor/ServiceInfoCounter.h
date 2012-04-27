/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_SERVICEINFOCOUNTER_H
#define _GGS_GAMEEXECUTOR_SERVICEINFOCOUNTER_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/Enum.h>
#include <GameExecutor/ServiceInfo.h>

#include <Settings/Settings>

#include <Core/Service>

#include <QtCore/QMutex>

using GGS::Settings::Settings;

namespace GGS {
  namespace GameExecutor {
    class GAMEEXECUTOR_EXPORT ServiceInfoCounter : public QObject
    {
      Q_OBJECT
    public:
      ServiceInfoCounter(QObject *parent = 0);
      ~ServiceInfoCounter();

      static ServiceInfo queryInfo(const Core::Service &service);

    public slots:
      void started(const Core::Service &service);
      void finished(const Core::Service &service, GGS::GameExecutor::FinishState state);

    private:
      Settings::Settings _storage;
      QMutex _lock;
    };
  }
}

#endif // _GGS_GAMEEXECUTOR_SERVICEINFOCOUNTER_H
