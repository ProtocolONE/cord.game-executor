#include <GameExecutor/ExecutorBase.h>

namespace GGS {
  namespace GameExecutor {
    ExecutorBase::ExecutorBase(QObject *parent /*= 0*/)
    {
    }

    ExecutorBase::ExecutorBase(const QString &scheme, QObject *parent /*= 0*/)
      : _scheme(scheme)
    {
    }

    ExecutorBase::~ExecutorBase()
    {
    }

    const QString &ExecutorBase::scheme() const
    {
      return this->_scheme;
    }
  }
};