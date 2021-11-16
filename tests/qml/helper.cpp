
#include "helper.h"
#include "../../src/mozillavpn.h"

TestHelper* TestHelper::instance() {
  static TestHelper* s_instance = new TestHelper();
  return s_instance;
}
TestHelper::TestHelper() : m_private(new Private()) {}

void TestHelper::triggerInitializeGlean() {
  emit MozillaVPN::instance()->initializeGlean();
}

void TestHelper::setGleanSourceTags(const QStringList& tags) {
  emit MozillaVPN::instance()->setGleanSourceTags(tags);
}