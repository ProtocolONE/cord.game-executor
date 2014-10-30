/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef EXECUTORMOCK_H
#define EXECUTORMOCK_H

#include <GameExecutor/ExecutorBase.h>

#include <RestApi/GameNetCredential.h>

#include <Core/Service>
#include <QObject>

using namespace GGS;
using namespace GGS::GameExecutor;

class ExecutorMock : public ExecutorBase
{
  Q_OBJECT

public:
    ExecutorMock(QObject *parent = 0);
    ~ExecutorMock();

    virtual void execute(
      const GGS::Core::Service &service, 
      GameExecutorService *executorService,
      const GGS::RestApi::GameNetCredential& credential,
      const GGS::RestApi::GameNetCredential& secondCredential) override;

    GGS::RestApi::GameNetCredential credential;
    GGS::RestApi::GameNetCredential secondCredential;
};

#endif // EXECUTORMOCK_H
