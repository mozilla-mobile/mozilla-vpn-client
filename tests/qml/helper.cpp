
#include "helper.h"
#include "../../src/logger.h"
#include "../../src/mozillavpn.h"

namespace {
Logger logger("Test", "Helper");
}

TestHelper* TestHelper::instance() {
  static TestHelper* s_instance = new TestHelper();
  return s_instance;
}
TestHelper::TestHelper() : m_private(new Private()) {}

void TestHelper::triggerInitializeGlean() {
  logger.debug() << "TESTING - triggerInitializeGlean";
  MozillaVPN* mvpn = MozillaVPN::instance();
  emit mvpn->initializeGlean();
}

void TestHelper::setGleanSourceTags(const QStringList& tags) {
  logger.debug() << "TESTING - setGleanSourceTags";
  MozillaVPN* mvpn = MozillaVPN::instance();
  emit mvpn->setGleanSourceTags(tags);
}