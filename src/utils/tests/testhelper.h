/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QByteArray>
#include <QObject>
#include <QString>

class TestRegistration {
 public:
  explicit TestRegistration(const QMetaObject* meta) {
    m_meta = meta;
    m_next = s_listhead;
    s_listhead = this;
  }

  // Test list iteration
  const QMetaObject* metaObject() const { return m_meta; }
  const TestRegistration* next() const { return m_next; }
  static const TestRegistration* first() { return s_listhead; }

 private:
  const QMetaObject* m_meta;
  const TestRegistration* m_next;

  static inline TestRegistration* s_listhead = nullptr;
};

// Test classes should inherit this to register themselves.
template <typename T>
class TestHelper {
 protected:
  // Try to coerce the linker to prevent s_registration gets included.
  explicit TestHelper() { Q_UNUSED(s_registration); };

 private:
  class TestRegistrationHelper : public TestRegistration {
   public:
    TestRegistrationHelper() : TestRegistration(&T::staticMetaObject){};
  };
  static inline TestRegistrationHelper s_registration;
};

#endif  // TESTHELPER_H
