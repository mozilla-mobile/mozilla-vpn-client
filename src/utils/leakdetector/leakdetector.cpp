/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utils/leakdetector/leakdetector.h"

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QTextStream>

#include "leakdetector.h"

#ifdef MZ_DEBUG
static QMutex s_leakDetector;

QHash<QString, QHash<void*, uint32_t>> s_leaks;
#endif

LeakDetector::LeakDetector() {
#ifndef MZ_DEBUG
  qFatal("LeakDetector _must_ be created in debug builds only!");
#endif
}

LeakDetector::~LeakDetector() {
#ifdef MZ_DEBUG
  if (!m_out) {
    return;
  }
  (*m_out) << "== MZ  - Leak report ===================" << Qt::endl;

  bool hasLeaks = false;
  for (auto i = s_leaks.begin(); i != s_leaks.end(); ++i) {
    QString className = i.key();

    if (i->size() == 0) {
      continue;
    }

    hasLeaks = true;
    (*m_out) << className << Qt::endl;

    for (auto l = i->begin(); l != i->end(); ++l) {
      (*m_out) << "  - ptr: " << l.key() << " size:" << l.value() << Qt::endl;
    }
  }

  if (!hasLeaks) {
    (*m_out) << "No leaks detected." << Qt::endl;
  }

  (*m_out).flush();
#endif
}

#ifdef MZ_DEBUG
void LeakDetector::logCtor(void* ptr, const char* typeName, uint32_t size) {
  QMutexLocker lock(&s_leakDetector);

  QString type(typeName);
  if (!s_leaks.contains(type)) {
    s_leaks.insert(type, QHash<void*, uint32_t>());
  }

  s_leaks[type].insert(ptr, size);
}

void LeakDetector::logDtor(void* ptr, const char* typeName, uint32_t size) {
  QMutexLocker lock(&s_leakDetector);

  QString type(typeName);
  Q_ASSERT(s_leaks.contains(type));

  QHash<void*, uint32_t>& leak = s_leaks[type];
  Q_ASSERT(leak.contains(ptr));
  Q_ASSERT(leak[ptr] == size);
  leak.remove(ptr);
}
#endif

const QHash<QString, QHash<void*, uint32_t>> LeakDetector::getObjects() {
#ifdef MZ_DEBUG
  return s_leaks;
#else
  return {};
#endif
}

void LeakDetector::setOutStream(std::unique_ptr<QTextStream> other) {
  m_out.swap(other);
}
