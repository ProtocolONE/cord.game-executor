#include <Windows.h>
#include <QtCore/QMetaType>

#include <GameExecutor/Enum.h>

namespace P1 {
  namespace GameExecutor {

    void registerTypes()
    {
      qRegisterMetaType<P1::GameExecutor::FinishState>("P1::GameExecutor::FinishState");
    }
  }
}

