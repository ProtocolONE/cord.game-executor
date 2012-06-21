#include <GameExecutor/Hook/RestoreResolution.h>
#include <QtCore/QDebug>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {
      RestoreResolution::RestoreResolution(QObject *parent/*=0*/) : HookInterface(parent)
      {
      }

      RestoreResolution::~RestoreResolution()
      {
      }

      void RestoreResolution::PreExecute(Core::Service &service)
      {
        DEBUG_LOG << "for" << service.id();

        //http://msdn.microsoft.com/en-us/library/dd162611(v=vs.85).aspx
        this->_enabled = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &this->_beforeExecuteDisplay);
        emit this->preExecuteCompleted(GGS::GameExecutor::Success);
      }

      void RestoreResolution::PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state)
      {
        DEBUG_LOG << "for" << service.id();

        if (!this->_enabled) {
          emit this->postExecuteCompleted();
          return;
        }

        DEVMODE afterExecuteDisplay;
        bool result = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &afterExecuteDisplay);
        if (!result) {
          emit this->postExecuteCompleted();
          return;
        }

        if (afterExecuteDisplay.dmPelsHeight == this->_beforeExecuteDisplay.dmPelsHeight 
          && afterExecuteDisplay.dmPelsWidth == this->_beforeExecuteDisplay.dmPelsWidth
          && afterExecuteDisplay.dmBitsPerPel == this->_beforeExecuteDisplay.dmBitsPerPel) {
            emit this->postExecuteCompleted();
            return;
        }

        //http://msdn.microsoft.com/en-us/library/dd183411(v=vs.85).aspx
        if (DISP_CHANGE_FAILED == ChangeDisplaySettings(&this->_beforeExecuteDisplay, CDS_RESET)) {
           emit this->postExecuteCompleted();
           return;
        }

        ChangeDisplaySettings(&this->_beforeExecuteDisplay, CDS_UPDATEREGISTRY);
        emit this->postExecuteCompleted();
      }
    }
  }
}

