/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef EXECUTORWRAPPER_H
#define EXECUTORWRAPPER_H

#include <GameExecutor/ExecutorBase.h>

#include <Core/Service>

#include <functional>
#include <QObject>

using namespace GGS;

typedef std::tr1::function<void(const Core::Service &)> ExecutorStartedFunc;
typedef std::tr1::function<void(const Core::Service &, GGS::GameExecutor::FinishState)> ExecutorFinishedFunc;

class ExecutorWrapper : public QObject
{
  Q_OBJECT

public:
  ExecutorWrapper(QObject *parent = 0);
  ExecutorWrapper(GGS::GameExecutor::ExecutorBase *executor);
  ~ExecutorWrapper();

  void setStarted(ExecutorStartedFunc func);
  void setFinished(ExecutorFinishedFunc func);
  void setExecutor(GGS::GameExecutor::ExecutorBase *executor);

private slots:
  void started(const Core::Service &service);
  void finished(const Core::Service &service, GGS::GameExecutor::FinishState state);

private:
  ExecutorStartedFunc _startedFunc;
  ExecutorFinishedFunc _finishedFunc;
};

#endif // EXECUTORWRAPPER_H

