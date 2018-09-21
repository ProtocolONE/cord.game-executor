#include <GameExecutor/GameExecutorService.h>

#include <GameExecutor/Executor/ExecutorMock.h>
#include <GameExecutor/Hooks/HookMock.h>
#include <GameExecutor/GameExecutorServiceWrapper.h>

#include <Core/Service.h>

#include <gtest/gtest.h>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QSignalSpy>

using P1::GameExecutor::GameExecutorService;

class GameExecutorServiceTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    executorService.registerExecutor(&mockExecutor);

    ASSERT_TRUE(executorService.hasExecutor("test"));

    service.setId("id");
    service.setUrl(QUrl("test://"));

    P1::RestApi::RestApiManager::setCommonInstance(&this->restapiManger);
  }

  virtual void TearDown() 
  {
  }

  ExecutorMock mockExecutor;
  P1::RestApi::RestApiManager restapiManger;
  P1::RestApi::ProtocolOneCredential defaultCredential;
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

  wrapper.setFinished([&](const Core::Service &service, P1::GameExecutor::FinishState state) {
    check.append("finish");
    loop.exit();
  });
  
  //Это может показаться странным, но это лучший способ проверить корректность следования сигналов при условии, что
  //часть сигналы эмитятся в разных потоках.
  for (int y = 0; y < 1000; y++) {
    executorService.execute(service, defaultCredential);

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
  
  hook.setCanFunc([](const Core::Service &service) -> P1::GameExecutor::FinishState {
    return P1::GameExecutor::CanExecutionHookBreak;
  });
  
  executorService.clearHooks(service);
  executorService.addHook(service, &hook);

  QSignalSpy canExecute(&executorService, SIGNAL(canExecuteCompleted(const P1::Core::Service)));
  QSignalSpy preExecute(&executorService, SIGNAL(preExecuteCompleted(const P1::Core::Service)));
  QSignalSpy startExecute(&executorService, SIGNAL(started(const P1::Core::Service)));
  QSignalSpy finishExecute(&executorService, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const P1::Core::Service &service, P1::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  executorService.execute(service, defaultCredential);
  loop.exec();

  ASSERT_EQ(0, canExecute.count());
  ASSERT_EQ(0, preExecute.count());
  ASSERT_EQ(0, startExecute.count());
  ASSERT_EQ(P1::GameExecutor::CanExecutionHookBreak, 
            finishExecute.at(0).at(1).value<P1::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, PreExecuteStopTheLoop)
{
  HookMock hook;

  hook.setPreFunc([](const Core::Service &service) -> P1::GameExecutor::FinishState {
    return P1::GameExecutor::PreExecutionHookBreak;
  });

  executorService.clearHooks(service);
  executorService.addHook(service, &hook);

  QSignalSpy canExecute(&executorService, SIGNAL(canExecuteCompleted(const P1::Core::Service)));
  QSignalSpy preExecute(&executorService, SIGNAL(preExecuteCompleted(const P1::Core::Service)));
  QSignalSpy startExecute(&executorService, SIGNAL(started(const P1::Core::Service)));
  QSignalSpy finishExecute(&executorService, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const P1::Core::Service &service, P1::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  executorService.execute(service, defaultCredential);
  loop.exec();

  ASSERT_EQ(1, canExecute.count());
  ASSERT_EQ(0, preExecute.count());
  ASSERT_EQ(0, startExecute.count());
  ASSERT_EQ(P1::GameExecutor::PreExecutionHookBreak, 
            finishExecute.at(0).at(1).value<P1::GameExecutor::FinishState>());
}

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

#define NORMAL_FLOW_OF_THE_LOOP_HOOK(Name,Index) \
  HookMock Name;\
  Name.setCanFunc([&](const Core::Service &service) -> P1::GameExecutor::FinishState {\
    check.append(QString(STRINGIZE(PPCAT(can, Name))));\
    return P1::GameExecutor::Success;\
  });\
  Name.setPreFunc([&](const Core::Service &service) -> P1::GameExecutor::FinishState {\
   check.append(QString(STRINGIZE(PPCAT(pre, Name))));\
   return P1::GameExecutor::Success; \
  });\
  Name.setPostFunc([&](const Core::Service &service, P1::GameExecutor::FinishState state) { \
   check.append(QString(STRINGIZE(PPCAT(post, Name))));\
  });\
  executorService.addHook(service, &Name, Index);

TEST_F(GameExecutorServiceTest, NormalFlowOfTheLoop)
{
  QStringList check;
  
  QSignalSpy canExecuteSpy(&executorService, SIGNAL(canExecuteCompleted(const P1::Core::Service)));
  QSignalSpy preExecuteSpy(&executorService, SIGNAL(preExecuteCompleted(const P1::Core::Service)));
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const P1::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));

  executorService.clearHooks(service);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook1, 0);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook2, 0);
  NORMAL_FLOW_OF_THE_LOOP_HOOK(Hook3, 1);
  
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const P1::Core::Service &service, P1::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  executorService.execute(service, defaultCredential);
  loop.exec();

  //UNDONE Проверить, что Core::Service реально тот, что был передан
  ASSERT_EQ(1, canExecuteSpy.at(0).count());
  ASSERT_EQ(1, preExecuteSpy.at(0).count());
  ASSERT_EQ(1, startedSpy.count());
  ASSERT_EQ(P1::GameExecutor::Success, finishedSpy.at(0).at(1).value<P1::GameExecutor::FinishState>());

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
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const P1::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));

  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const P1::Core::Service &service, P1::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  Core::Service srv;
  srv.setUrl(QUrl("test://"));

  executorService.execute(srv, defaultCredential);
  loop.exec();

  ASSERT_EQ(0, startedSpy.count());
  ASSERT_EQ(P1::GameExecutor::InvalidService, 
            finishedSpy.at(0).at(1).value<P1::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, UnknownSchemeErrorCase) 
{
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const P1::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));
 
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([this](const P1::Core::Service &service, P1::GameExecutor::FinishState state) {
    this->loop.exit();
  });

  service.setUrl(QUrl("wrongScheme://"));

  executorService.execute(service, defaultCredential);
  loop.exec();

  ASSERT_EQ(0, startedSpy.count());
  ASSERT_EQ(P1::GameExecutor::UnknownSchemeError, 
            finishedSpy.at(0).at(1).value<P1::GameExecutor::FinishState>());
}

TEST_F(GameExecutorServiceTest, MultiStartErrorCase) 
{
  QSignalSpy startedSpy(&executorService, SIGNAL(started(const P1::Core::Service)));
  QSignalSpy finishedSpy(&executorService, SIGNAL(finished(const P1::Core::Service, P1::GameExecutor::FinishState)));

  int count = 0;
  GameExecutorServiceWrapper wrapper(&executorService);
  wrapper.setFinished([&](const Core::Service &service, P1::GameExecutor::FinishState state) {
    if (++count == 2) 
      loop.exit();
  });

  service.setUrl(QUrl("test://"));
  executorService.execute(service, defaultCredential);
  executorService.execute(service, defaultCredential);
  loop.exec();

  ASSERT_EQ(1, startedSpy.count());
  ASSERT_EQ(2, finishedSpy.count());

  //Спервы мы получим ошибку от повторного запуска
  EXPECT_EQ(P1::GameExecutor::AlreadyStartedError, 
    finishedSpy.at(0).at(1).value<P1::GameExecutor::FinishState>());
  
  //Потом корректный ответ от первого запуска
  EXPECT_EQ(P1::GameExecutor::Success, 
    finishedSpy.at(1).at(1).value<P1::GameExecutor::FinishState>());
}
