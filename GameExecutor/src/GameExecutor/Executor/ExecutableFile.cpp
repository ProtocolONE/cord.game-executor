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
#include <GameExecutor/Executor/ExecutableFile.h>
#include <GameExecutor/Executor/ExecutableFile_p.h>

namespace GGS {
  namespace GameExecutor {
    namespace Executor {
      ExecutableFile::ExecutableFile(QObject *parent) 
        : ExecutorBase("exe", parent)
      {
      }

      ExecutableFile::~ExecutableFile()
      {
      }

      void ExecutableFile::execute(
        const GGS::Core::Service &service, 
        GameExecutorService *executorService,
        const GGS::RestApi::GameNetCredential& credential,
        const GGS::RestApi::GameNetCredential& secondCredential)
      {
        QString id = service.id();
        ExecutableFilePrivate* executor = new ExecutableFilePrivate(this);

        QObject::connect(executor, &ExecutableFilePrivate::started,
          this, &ExecutableFile::started, Qt::DirectConnection);

        QObject::connect(executor, &ExecutableFilePrivate::finished,
          this, &ExecutableFile::internalFinished, Qt::DirectConnection);

        QObject::connect(executorService, &GameExecutorService::stopExecution,
          executor, &ExecutableFilePrivate::shutdown);

        executor->execute(service, executorService, credential, secondCredential);
      }

      void ExecutableFile::internalFinished(const GGS::Core::Service &service, GGS::GameExecutor::FinishState state)
      {
        QObject::sender()->deleteLater();
        emit this->finished(service, state);
      }
    }
  }
}