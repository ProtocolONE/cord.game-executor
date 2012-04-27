/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEEXECUTOR_HOOKINTERFACE_H
#define _GGS_GAMEEXECUTOR_HOOKINTERFACE_H

#include <GameExecutor/gameexecutor_global.h>
#include <GameExecutor/Enum.h>

#include <Core/Service>

namespace GGS {
  namespace GameExecutor {

    /*!
      \class HookInterface
    
      \brief ��������� ���������� �������� ���������� ��������� ������� ��������.
    */
    class GAMEEXECUTOR_EXPORT HookInterface
    {
    public:
      virtual ~HookInterface() {};

      /*!
        \fn virtual bool HookInterface::CanExecute(const Core::Service &service) = 0;
      
        \brief ���������� ����� �� ���� ������� ������.
              
        \param service The service.
      
        \return true, ���� ������ ����� ���� �������.
      */
      virtual bool CanExecute(const Core::Service &service) = 0;

      /*!
        \fn virtual bool HookInterface::PreExecute(const Core::Service &service) = 0;
      
        \brief ����������� ��������������� ����� �������� �������. ����� ����������� ���������� ������ �������,
               ���� ������ false. 
      
        \param service The service.
      
        \return true, ���� ������ ����� ���� �������.
      */
      virtual bool PreExecute(const Core::Service &service) = 0;

      /*!
        \fn virtual void HookInterface::PostExecute(const Core::Service &service,
          GGS::GameExecutor::FinishState state) = 0;
      
        \brief ����������� �� ���������� ������ �������. � ������� GGS::GameExecutor::FinishState ������� ����� ��
               ���������� ��� ������� �������.
      
        \param service The service.
        \param state   The state.

        \sa GGS::GameExecutor::FinishState
      */
      virtual void PostExecute(const Core::Service &service, GGS::GameExecutor::FinishState state) = 0;
    };
  }
}

#endif _GGS_GAMEEXECUTOR_HOOKINTERFACE_H