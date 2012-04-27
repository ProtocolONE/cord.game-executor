#include <GameExecutor/ExecutorBase.h>

namespace GGS {
  namespace GameExecutor {
    const QString &ExecutorBase::scheme() const
    {
      return this->_scheme;
    }

    ExecutorBase::ExecutorBase(QObject *parent /*= 0*/)
    {
    }

    ExecutorBase::~ExecutorBase()
    {
    }
  }
};