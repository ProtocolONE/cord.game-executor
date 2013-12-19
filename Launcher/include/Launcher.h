/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef LAUNCHER_LAUNCHER_H
#define LAUNCHER_LAUNCHER_H

#include <GameExecutor/Executor/ExecutableFileClient.h>

#include <RestApi/RestApiManager>
#include <RestApi/HttpCommandRequest>
#include <RestApi/FakeCache>

#include <QObject>
#include <QtCore/QCoreApplication>

class Launcher : public QObject
{
  Q_OBJECT

public:
  Launcher(QObject *parent = 0);
  ~Launcher();

  void exec(QCoreApplication &app);
  GGS::GameExecutor::Executor::ExecutableFileClient client;

private slots:
  void exit(int code);

private:
  GGS::RestApi::HttpCommandRequest request;
  GGS::RestApi::FakeCache cache;
  GGS::RestApi::RestApiManager restapi;
  
};

#endif // LAUNCHER_LAUNCHER_H
