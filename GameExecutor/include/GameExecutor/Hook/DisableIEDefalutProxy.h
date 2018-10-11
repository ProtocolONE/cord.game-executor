#pragma once

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/HookInterface.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameExecutor {
    namespace Hook {

      /*!
        \class DisableIEDefalutProxy
      
        \brief ���������� ������ ����� �������� ����. ��������� ��� Aika2, ������� �� ���� �� �� �����������, ���� 
        � InternetExplorer ���������� ����� ������.
      */
      class GAMEEXECUTOR_EXPORT DisableIEDefalutProxy : public HookInterface
      {
        Q_OBJECT
      public:
        explicit DisableIEDefalutProxy(QObject *parent = 0);
        virtual ~DisableIEDefalutProxy();

        static QString id();

        virtual void PreExecute(Core::Service &service);
      };
    }
  }
}
