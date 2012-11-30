#include <FakeCaller.h>

void FakeCaller::exec()
{
  emit this->caller(this->_message);
}

