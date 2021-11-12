
#include "helper.h"

TestHelper* TestHelper::instance() {
  static TestHelper* s_instance = new TestHelper();
  return s_instance;
}
TestHelper::TestHelper() : m_private(new Private()) {}