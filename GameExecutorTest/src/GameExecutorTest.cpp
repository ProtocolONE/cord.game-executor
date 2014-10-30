/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
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

    GGS::RestApi::RestApiManager::setCommonInstance(&this->restapiManger);
  }

  virtual void TearDown() 
  {
  }

  ExecutorMock mockExecutor;
  GGS::RestApi::RestApiManager restapiManger;
  GGS::RestApi::GameNetCredential defaultCredential;
  GameExecutorService executorService;
  Core::Service service;

  QEventLoop loop;
};

TEST_F(GameExecutorServiceTest, BaseSignals)
{
  QStringList check;

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setCanExecuteCompleted([&](const Core::Service &service) {
    check.append("can");
  });

  wrapper.setPreExecuteCompleted([&](const Core::Service &service) {
    check.append("pre");
  });

  wrapper.setStarted([&](const Core::Service &service) {
    check.append("start");
  });

  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    check.append("finish");
    loop.exit();
  });
  
  //Это может показаться странным, но это лучший способ проверить корректность следования сигналов при условии, что
  //часть сигналы эмитятся в разных потоках.
  for (int y = 0; y < 1000; y++) {
    executorService.executeEx(service, defaultCredential);

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
  
  hook.setCanFunc([](const Core::Service &service) -> GGS::GameExecutor::FinishState {
    return GGS::GameExecutor::CanExecutionHookBreak;
  });
  
  executorService.clearHooks(service);
  executorService.addHook(service, &hook);

  QSignalSpy canExecute(&executorService, SIGNAL(canExecuteCompleted(const GGS::Core::Service)));
  QSignalSpy preExecute(&executorService, SIGNAL(preExecuteCompleted(const GGS::Core::Service)));
  QSignalSpy startExecute(&executorService, SIGNAL(started(const GGS::Core::Service)));
  QSignalSpy finishExecute(&executorService, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  executorService.executeEx(service, defaultCredential);
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

  hook.setPreFunc([](const Core::Service &service) -> GGS::GameExecutor::FinishState {
    return GGS::GameExecutor::PreExecutionHookBreak;
  });

  executorService.clearHooks(service);
  executorService.addHook(service, &hook);

  QSignalSpy canExecute(&executorService, SIGNAL(canExecuteCompleted(const GGS::Core::Service)));
  QSignalSpy preExecute(&executorService, SIGNAL(preExecuteCompleted(const GGS::Core::Service)));
  QSignalSpy startExecute(&executorService, SIGNAL(started(const GGS::Core::Service)));
  QSignalSpy finishExecute(&executorService, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  executorService.executeEx(service, defaultCredential);
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
  Name.setCanFunc([&](const Core::Service &service) -> GGS::GameExecutor::FinishState {\
    check.append(QString(STRINGIZE(PPCAT(can, Name))));\
    return GGS::GameExecutor::Success;\
  });\
  Name.setPreFunc([&](const Core::Service &service) -> GGS::GameExecutor::FinishState {\
   check.append(QString(STRINGIZE(PPCAT(pre, Name))));\
   return GGS::GameExecutor::Success; \
  });\
  Name.setPostFunc([&](const Core::Service &service, GGS::GameExecutor::FinishState state) { \
   check.append(QString(STRINGIZE(PPCAT(post, Name))));\
  });\
  executorService.addHook(service, &Name, Index);

TEST_F(GameExecutorServiceTest, NormalFlowOfTheLoop)
{
  QStringList check;
  
  QSignalSpy canExecuteSpy(&executorService, SIGNAL(canExecuteCompleted(const GGS::Core::Service)));
  QSignalSpy preExecuteSpy(&executorService, SIGNAL(preExecuteCompleted(const GGS::Core::Service)));
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const GGS::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

  executorService.clearHooks(service);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook1, 0);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook2, 0);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook3, 1);
  
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  executorService.executeEx(service, defaultCredential);
  loop.exec();

  //UNDONE Проверить, что Core::Service реально тот, что был передан
  ASSERT_EQ(1, canExecuteSpy.at(0).count());
  ASSERT_EQ(1, preExecuteSpy.at(0).count());
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
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const GGS::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  Core::Service srv;
  srv.setUrl(QUrl("test://"));

  executorService.executeEx(srv, defaultCredential);
  loop.exec();

  ASSERT_EQ(0, startedSpy.count());
  ASSERT_EQ(GGS::GameExecutor::InvalidService, 
            finishedSpy.at(0).at(1).value<GGS::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, UnknownSchemeErrorCase) 
{
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const GGS::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));
 
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const GGS::Core::Service &service, GGS::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  service.setUrl(QUrl("wrongScheme://"));

  executorService.executeEx(service, defaultCredential);
  loop.exec();

  ASSERT_EQ(0, startedSpy.count());
  ASSERT_EQ(GGS::GameExecutor::UnknownSchemeError, 
            finishedSpy.at(0).at(1).value<GGS::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, MultiStartErrorCase) 
{
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const GGS::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const GGS::Core::Service, GGS::GameExecutor::FinishState)));

  int count = 0;
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    if (++count == 2) 
      loop.exit();
  });

  service.setUrl(QUrl("test://"));
  executorService.executeEx(service, defaultCredential);
  executorService.executeEx(service, defaultCredential);
  loop.exec();

  ASSERT_EQ(1, startedSpy.count());
  ASSERT_EQ(2, finishedSpy.count());

  //Спервы мы получим ошибку от повторного запуска
  EXPECT_EQ(GGS::GameExecutor::AlreadyStartedError, 
    finishedSpy.at(0).at(1).value<GGS::GameExecutor::FinishState>());
  
  //Потом корректный ответ от первого запуска
  EXPECT_EQ(GGS::GameExecutor::Success, 
    finishedSpy.at(1).at(1).value<GGS::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, TestNewCredential)
{
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const Core::Service &service, GGS::GameExecutor::FinishState state) {
    this->loop.exit();
  });
  
  GGS::RestApi::GameNetCredential oldCredential;
  oldCredential.setUserId("11111111111111111");
  oldCredential.setAppKey("aaaaaaaaaaaaaaaaa");
  oldCredential.setCookie("ccccccccccccccccc");

  GGS::RestApi::RestApiManager::commonInstance()->setCridential(oldCredential);

  GGS::RestApi::GameNetCredential mainCredential;
  mainCredential.setUserId("22222222222222222");
  mainCredential.setAppKey("bbbbbbbbbbbbbbbbb");
  mainCredential.setCookie("ddddddddddddddddd");

  GGS::RestApi::GameNetCredential secondCredential;
  secondCredential.setUserId("33333333333333333");
  secondCredential.setAppKey("eeeeeeeeeeeeeeeee");
  secondCredential.setCookie("fffffffffffffffff");

  executorService.executeEx(service, mainCredential, secondCredential);
  loop.exec();

  ASSERT_EQ(mainCredential.userId(), mockExecutor.credential.userId());
  ASSERT_EQ(mainCredential.appKey(), mockExecutor.credential.appKey());
  ASSERT_EQ(mainCredential.cookie(), mockExecutor.credential.cookie());

  ASSERT_EQ(secondCredential.userId(), mockExecutor.secondCredential.userId());
  ASSERT_EQ(secondCredential.appKey(), mockExecutor.secondCredential.appKey());
  ASSERT_EQ(secondCredential.cookie(), mockExecutor.secondCredential.cookie());
}