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
  TestRegistration(const QMetaObject* meta) {
    m_meta = meta;
    m_next = s_listhead;
    s_listhead = this;
  }
  const QMetaObject* m_meta;
  const TestRegistration* m_next;

  static const TestRegistration* list() { return s_listhead; }

 private:
  static inline TestRegistration* s_listhead = nullptr;
};

// Test classes should inherit this to register themselves.
template<typename T>
class TestHelper : private TestRegistration {
 public:
  // Helpers to convert long hex to byte arrays.
  QByteArray fromHex(const QString& str) {
    return QByteArray::fromHex(str.toUtf8());
  };
  template<typename... Args>
  QByteArray fromHex(const QString& str, Args... args) {
    return fromHex(str) + fromHex(args...);
  }

 protected:
  TestHelper() : TestRegistration(&T::staticMetaObject) {
    // Try to coerce the linker to ensure s_registration gets included
    Q_UNUSED(s_registration);
  };

 private:
  static inline TestHelper<T> s_registration;
};

#endif  // TESTHELPER_H
