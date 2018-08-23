#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/Enum.h>

#include <RestApi/GameNetCredential.h>

#include <Core/Service.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameExecutor {

    /*!
      \class HookInterface
    
      \brief Интерфейс определяет механизм расширения процедуры запуска сервисов.
    */
    class GAMEEXECUTOR_EXPORT HookInterface : public QObject
    {
      Q_OBJECT
    public:
      explicit HookInterface(QObject *parent = 0);
      virtual ~HookInterface();

      /*!
        \fn virtual bool HookInterface::CanExecute(const Core::Service &service) = 0;
      
        \brief Определяет может ли быть запущен сервис.
              
        \param service The service.
      
        \return true, если сервис может быть запущен.
      */
      virtual void CanExecute(Core::Service &service);

      /*!
        \fn virtual bool HookInterface::PreExecute(const Core::Service &service) = 0;
      
        \brief Выполняется непосредственно перед запуском сервиса. Имеет возможность остановить запуск сервиса,
               если вернут false. 
      
        \param service The service.
      
        \return true, если сервис может быть запущен.
      */
      virtual void PreExecute(Core::Service &service);

      /*!
        \fn virtual void HookInterface::PostExecute(const Core::Service &service,
          P1::GameExecutor::FinishState state) = 0;
      
        \brief Выполняется по завершению работы сервиса. С помощью P1::GameExecutor::FinishState функция узнаёт об
               успешности или неудаче запуска.
      
        \param service The service.
        \param state   The state.

        \sa P1::GameExecutor::FinishState
      */
      virtual void PostExecute(Core::Service &service, P1::GameExecutor::FinishState state);

      /**
       * \fn  void HookInterface::setCredential(const P1::RestApi::GameNetCredential& value);
       *
       * \brief Установить авторизацию, с которой запускается игра.
       *
       * \author  Ilya Tkachenko
       * \date  08.09.2014
       *
       * \param value Авторизация пользователя.
       */

      void setCredential(const P1::RestApi::GameNetCredential& value);

    signals:
      void canExecuteCompleted(const P1::Core::Service &service, P1::GameExecutor::FinishState result);
      void preExecuteCompleted(const P1::Core::Service &service, P1::GameExecutor::FinishState result);
      void postExecuteCompleted(const P1::Core::Service &service);

    protected:
      P1::RestApi::GameNetCredential _credential;
    };
  }
}
