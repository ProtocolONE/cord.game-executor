/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef HOOKMOCK_H
#define HOOKMOCK_H

#include <GameExecutor/HookInterface.h>
#include <Core/Service>

#include <functional>
#include <QObject>

using namespace GGS;
using namespace GGS::GameExecutor;

typedef std::tr1::function<bool(const Core::Service &)> HookCanPreFunc;
typedef std::tr1::function<void(const Core::Service &, GGS::GameExecutor::FinishState)> HookPostFunc;

class HookMock : public QObject, public GGS::GameExecutor::HookInterface
{
  Q_OBJECT

public:
  HookMock(QObject *parent = 0);
  ~HookMock();

  bool CanExecute( const Core::Service &service );

  void PostExecute( const Core::Service &service, GGS::GameExecutor::FinishState state );

  bool PreExecute( const Core::Service &service );

  void setCanFunc(HookCanPreFunc func);
  void setPreFunc(HookCanPreFunc func);
  void setPostFunc(HookPostFunc func);
private:

  HookCanPreFunc _canFunc;
  HookCanPreFunc _preFunc;
  HookPostFunc _postFunc;
};

#endif // HOOKMOCK_H
