#include <GameExecutor/Executor/WebLinkSsl.h>

namespace P1 {
  namespace GameExecutor {
    namespace Executor {

      WebLinkSsl::WebLinkSsl(QObject *parent): WebLink(parent)
      {
         this->_scheme = QString("https");
      }

      WebLinkSsl::~WebLinkSsl()
      {
      }
    }
  }
}
