/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef GAMEEXECUTORSERVICEWRAPPER_H
#define GAMEEXECUTORSERVICEWRAPPER_H

#include <GameExecutor/GameExecutorService.h>

#include <functional>
#include <QObject>

typedef std::tr1::function<void(const GGS::Core::Service &)> CanPreFunc;
typedef std::tr1::function<void(const GGS::Core::Service &)> StartedFunc;
typedef std::tr1::function<void(const GGS::Core::Service &, GGS::GameExecutor::FinishState)> FinishedFunc;

class GameExecutorServiceWrapper : public QObject
{
  Q_OBJECT

public:
  explicit GameExecutorServiceWrapper(QObject *parent = 0);
  explicit GameExecutorServiceWrapper(GGS::GameExecutor::GameExecutorService *_service) ;
  ~GameExecutorServiceWrapper();

  void setGameExecutorService(GGS::GameExecutor::GameExecutorService *_service);

  void setCanExecuteCompleted(CanPreFunc func);
  void setPreExecuteCompleted(CanPreFunc func);
  void setStarted(StartedFunc func);
  void setFinished(FinishedFunc func);

private slots:
  void canExecuteCompleted(const GGS::Core::Service &service);
  void preExecuteCompleted(const GGS::Core::Service &service);
  void started(const GGS::Core::Service &service);
  void finished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state);

private:
  CanPreFunc _canExecuteCompletedFunc;
  CanPreFunc _preExecuteCompletedFunc;
  StartedFunc _startedFunc;
  FinishedFunc _finishedFunc;

  GGS::GameExecutor::GameExecutorService *_service;

};

#endif // GAMEEXECUTORSERVICEWRAPPER_H
