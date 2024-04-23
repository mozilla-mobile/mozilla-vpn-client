/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "testleakdetector.h"

#include "leakdetector.h"

class Dummy {
 public:
  Dummy() { MZ_COUNT_CTOR(Dummy); };
  ~Dummy() { MZ_COUNT_DTOR(Dummy); };
};

QTEST_MAIN(TestLeakDetector)

/**
 * Assert that a newly Created Object is
 * registered and can be retrieved.
 */
void TestLeakDetector::ctorRegisters() {
  auto x = std::make_unique<Dummy>();
  QVERIFY(LeakDetector::getObjects().value("Dummy").count() == 1);
  QVERIFY(LeakDetector::getObjects().value("Dummy").contains(
      static_cast<void*>(x.get())));
}

void TestLeakDetector::dtorRegisters() {
  auto x = std::make_unique<Dummy>();
  QVERIFY(LeakDetector::getObjects().value("Dummy").count() == 1);
  x.reset();
  QVERIFY(LeakDetector::getObjects().value("Dummy").count() == 0);
}
void TestLeakDetector::noLeakFound() {
  QString buffer;
  auto stream = std::make_unique<QTextStream>(&buffer, QTextStream::WriteOnly);
  auto detector = std::make_unique<LeakDetector>();
  detector->setOutStream(std::move(stream));

  // Now Make a Dummy and Propery Delete it
  auto dummy = std::make_unique<Dummy>();
  dummy.reset();
  // It will write the report on the destructor.
  detector.reset();
  QCOMPARE(buffer,
           "== MZ  - Leak report ===================\nNo leaks detected.\n");
}

void TestLeakDetector::leakFound() {
  QString buffer;
  auto stream = std::make_unique<QTextStream>(&buffer, QTextStream::WriteOnly);
  auto detector = std::make_unique<LeakDetector>();
  detector->setOutStream(std::move(stream));

  // Now Make a Dummy and keep it.
  auto dummy = std::make_unique<Dummy>();
  // It will write the report on the destructor.
  detector.reset();
  QVERIFY(buffer.startsWith(
      "== MZ  - Leak report ===================\nDummy\n  - ptr: 0x"));
}
