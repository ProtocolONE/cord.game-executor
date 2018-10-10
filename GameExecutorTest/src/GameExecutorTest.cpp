/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/GameExecutorService.h>

#include <GameExecutor/Executor/ExecutorMock.h>
#include <GameExecutor/Hooks/HookMock.h>
#include <GameExecutor/GameExecutorServiceWrapper.h>

#include <Core/Service>

#include <gtest/gtest.h>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QSignalSpy>

using GGS::GameExecutor::GameExecutorService;

class GameExecutorServiceTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    executorService.registerExecutor(&mockExecutor);

    ASSERT_TRUE(executorService.hasExecutor("test"));

    service.setId("id");
    service.setUrl(QUrl("test://"));
  }

  virtual void TearDown() 
  {
  }

  ExecutorMock mockExecutor;
  GameExecutorService executorService;
  Core::Service service;

  QEventLoop loop;
};

TEST_F(GameExecutorServiceTest, BaseSignals)
{
  QStringList check;

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setCanExecuteCompleted([&](const Core::Service &service, bool result) {
    check.append("can");
  });

  wrapper.setPreExecuteCompleted([&](const Core::Service &service, bool result) {
    check.append("pre");
  });

  wrapper.setStarted([&](const Core::Service &service) {
    check.append("start");
  });

  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    check.append("finish");
    loop.exit();
  });
  
  //��� ����� ���������� ��������, �� ��� ������ ������ ��������� ������������ ���������� �������� ��� �������, ���
  //����� ������� �������� � ������ �������.
  for (int y = 0; y < 1000; y++)
  {
    executorService.execute(service);

    loop.exec();

    ASSERT_EQ("can", check[0]);
    ASSERT_EQ("pre", check[1]);
    ASSERT_EQ("start", check[2]);
    ASSERT_EQ("finish", check[3]);

    check.clear();
  };
}

TEST_F(GameExecutorServiceTest, CanExecuteStopTheLoop)
{
  HookMock hook;
  
  hook.setCanFunc([&](const Core::Service &service) -> bool {
    return false;
  });
  
  executorService.clearHooks(service);
  executorService.addHook(service, &hook);

  QSignalSpy canExecute(&executorService, SIGNAL(canExecuteCompleted(const Core::Service, bool)));
  QSignalSpy preExecute(&executorService, SIGNAL(preExecuteCompleted(const Core::Service, bool)));
  QSignalSpy startExecute(&executorService, SIGNAL(started(const Core::Service)));
  QSignalSpy finishExecute(&executorService, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    loop.exit();
  });

  executorService.execute(service);
  loop.exec();

  ASSERT_EQ(0, canExecute.count());
  ASSERT_EQ(0, preExecute.count());
  ASSERT_EQ(0, startExecute.count());
  ASSERT_EQ(GGS::GameExecutor::CanExecutionHookBreak, 
            finishExecute.at(0).at(1).value<GGS::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, PreExecuteStopTheLoop)
{
  HookMock hook;

  hook.setPreFunc([&](const Core::Service &service) -> bool {
    return false;
  });

  executorService.clearHooks(service);
  executorService.addHook(service, &hook);

  QSignalSpy canExecute(&executorService, SIGNAL(canExecuteCompleted(const Core::Service, bool)));
  QSignalSpy preExecute(&executorService, SIGNAL(preExecuteCompleted(const Core::Service, bool)));
  QSignalSpy startExecute(&executorService, SIGNAL(started(const Core::Service)));
  QSignalSpy finishExecute(&executorService, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    loop.exit();
  });

  executorService.execute(service);
  loop.exec();

  ASSERT_EQ(1, canExecute.count());
  ASSERT_EQ(0, preExecute.count());
  ASSERT_EQ(0, startExecute.count());
  ASSERT_EQ(GGS::GameExecutor::PreExecutionHookBreak, 
            finishExecute.at(0).at(1).value<GGS::GameExecutor::FinishState>());
}

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

#define NORMAL_FLOW_OF_THE_LOOP_HOOK(Name,Index) \
  HookMock Name;\
  Name.setCanFunc([&](const Core::Service &service) -> bool {\
    check.append(QString(STRINGIZE(PPCAT(can, Name))));\
    return true;\
  });\
  Name.setPreFunc([&](const Core::Service &service) -> bool {\
   check.append(QString(STRINGIZE(PPCAT(pre, Name))));\
   return true; \
  });\
  Name.setPostFunc([&](const Core::Service &service, GGS::GameExecutor::FinishState state) { \
   check.append(QString(STRINGIZE(PPCAT(post, Name))));\
  });\
  executorService.addHook(service, &Name, Index);

TEST_F(GameExecutorServiceTest, NormalFlowOfTheLoop)
{
  QStringList check;
  
  QSignalSpy canExecuteSpy(&executorService, SIGNAL(canExecuteCompleted(const Core::Service, bool)));
  QSignalSpy preExecuteSpy(&executorService, SIGNAL(preExecuteCompleted(const Core::Service, bool)));
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)));

  executorService.clearHooks(service);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook1, 0);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook2, 0);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook3, 1);
  
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    loop.exit();
  });

  executorService.execute(service);
  loop.exec();

  //UNDONE ���������, ��� Core::Service ������� ���, ��� ��� �������
  ASSERT_EQ(true, canExecuteSpy.at(0).at(1).value<bool>());
  ASSERT_EQ(true, preExecuteSpy.at(0).at(1).value<bool>());
  ASSERT_EQ(1, startedSpy.count());
  ASSERT_EQ(GGS::GameExecutor::Success, finishedSpy.at(0).at(1).value<GGS::GameExecutor::FinishState>());

  ASSERT_EQ("canHook2", check[0]);
  ASSERT_EQ("canHook1", check[1]);
  ASSERT_EQ("canHook3", check[2]);
  ASSERT_EQ("preHook2", check[3]);
  ASSERT_EQ("preHook1", check[4]);
  ASSERT_EQ("preHook3", check[5]);
  ASSERT_EQ("postHook2", check[6]);
  ASSERT_EQ("postHook1", check[7]);
  ASSERT_EQ("postHook3", check[8]);
}

TEST_F(GameExecutorServiceTest, InvalidServiceCase) 
{
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    loop.exit();
  });

  Core::Service srv;
  srv.setUrl(QUrl("test://"));

  executorService.execute(srv);
  loop.exec();

  ASSERT_EQ(0, startedSpy.count());
  ASSERT_EQ(GGS::GameExecutor::InvalidService, 
            finishedSpy.at(0).at(1).value<GGS::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, UnknownSchemeErrorCase) 
{
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const Core::Service, GGS::GameExecutor::FinishState)));
 
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    loop.exit();
  });

  service.setUrl(QUrl("wrongScheme://"));

  executorService.execute(service);
  loop.exec();

  ASSERT_EQ(0, startedSpy.count());
  ASSERT_EQ(GGS::GameExecutor::UnknownSchemeError, 
            finishedSpy.at(0).at(1).value<GGS::GameExecutor::FinishState>());
}
