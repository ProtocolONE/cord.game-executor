#pragma once

#include <GameExecutor/HookInterface.h>
#include <Core/Service.h>

#include <functional>
#include <QObject>

using namespace P1;
using namespace P1::GameExecutor;

typedef std::function<P1::GameExecutor::FinishState(const Core::Service &)> HookCanPreFunc;
typedef std::function<void(const Core::Service &, P1::GameExecutor::FinishState)> HookPostFunc;

class HookMock : public P1::GameExecutor::HookInterface
{
  Q_OBJECT

public:
  HookMock(QObject *parent = 0);
  ~HookMock();

  void CanExecute(Core::Service &service );

  void PostExecute(Core::Service &service, P1::GameExecutor::FinishState state);

  void PreExecute(Core::Service &service );

  void setCanFunc(HookCanPreFunc func);
  void setPreFunc(HookCanPreFunc func);
  void setPostFunc(HookPostFunc func);
private:

  HookCanPreFunc _canFunc;
  HookCanPreFunc _preFunc;
  HookPostFunc _postFunc;
};
