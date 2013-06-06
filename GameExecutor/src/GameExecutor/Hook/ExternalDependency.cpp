<<<<<<< HEAD
/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

=======
>>>>>>> 9a72277... QGNA-293 Запуск установки редистра для МВ2 при ошибке запуска игры
#include <GameExecutor/Hook/ExternalDependency.h>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QFile>
<<<<<<< HEAD
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
=======
>>>>>>> 9a72277... QGNA-293 Запуск установки редистра для МВ2 при ошибке запуска игры

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      ExternalDependency::ExternalDependency(QObject *parent/*=0*/) : HookInterface(parent), _isInstalled(true)
      {
      }

      ExternalDependency::~ExternalDependency()
      {
      }

      void ExternalDependency::setExternalDepencyList(const QString& dependencyList)
      {
        this->_fileNames = dependencyList.split(',', QString::KeepEmptyParts);
      }

      void ExternalDependency::install(Core::Service &service) 
      {
        QProcess process;
        QStringList fileNames = this->_fileNames;

        DEBUG_LOG << "Start installing dependency";

        while (!fileNames.isEmpty()) {
          QString dependency = fileNames.takeFirst();
          QString args("");
          if (!fileNames.isEmpty())
            args = fileNames.takeFirst();

          QString fullPath = QString("%1/%2/Dependency/%3").arg(service.installPath(), service.areaString(), dependency);
          DEBUG_LOG << "install" << fullPath;

          if (!QFile::exists(fullPath)) {
            WARNING_LOG << "Dependency not found. Service " << service.id() << " Dependency: " << dependency;
            return;
          }

          QStringList argumentList = args.split(' ', QString::SkipEmptyParts);
          process.start(fullPath, argumentList);
          process.waitForFinished();

          emit this->postExecuteCompleted(service);
          emit this->externalDependencyInstalled(service.id());
        }
      }

      void ExternalDependency::PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state)
      {
        this->_isInstalled = !this->_isInstalled;
<<<<<<< HEAD
        QUrlQuery query(service.url());

        if (state == GGS::GameExecutor::ExternalFatalError &&
          query.queryItemValue("exitCode") == "1" &&
=======

        if (state == GGS::GameExecutor::ExternalFatalError &&
          service.url().queryItemValue("exitCode") == "1" &&
>>>>>>> 9a72277... QGNA-293 Запуск установки редистра для МВ2 при ошибке запуска игры
          !this->_isInstalled) {
            this->install(service);
            return;
        }

        emit this->postExecuteCompleted(service);
      }
    }
  }
}

