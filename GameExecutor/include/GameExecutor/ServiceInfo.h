/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_SERVICEINFO_H
#define _GGS_GAMEEXECUTOR_SERVICEINFO_H

#include <GameExecutor/gameexecutor_global.h>

#include <QtCore/QTime>
#include <QtCore/QDateTime>

namespace GGS {
  namespace GameExecutor {
    class GAMEEXECUTOR_EXPORT ServiceInfo
    {
    public:
      ServiceInfo(int successCount, int failedCount, int playedTime, const QDateTime &lastExecutionTime);
      ~ServiceInfo();

      int successCount() const;
      int failedCount() const;
      int playedTime() const;
      const QDateTime &lastExecutionTime() const;
    
    private:
      int _successCount;
      int _failedCount;
      int _playedTime;
      QDateTime _lastExecutionTime;
    };
  }
}
#endif //_GGS_GAMEEXECUTOR_SERVICEINFO_H



