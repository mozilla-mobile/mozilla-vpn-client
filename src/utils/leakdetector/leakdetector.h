/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LEAKDETECTOR_H
#define LEAKDETECTOR_H

#include <QObject>
#include <QTextStream>

#ifdef MZ_DEBUG
#  define MZ_COUNT_CTOR(_type)                                   \
    do {                                                         \
      static_assert(std::is_class<_type>(),                      \
                    "Token '" #_type "' is not a class type.");  \
      LeakDetector::logCtor((void*)this, #_type, sizeof(*this)); \
    } while (0)

#  define MZ_COUNT_DTOR(_type)                                   \
    do {                                                         \
      static_assert(std::is_class<_type>(),                      \
                    "Token '" #_type "' is not a class type.");  \
      LeakDetector::logDtor((void*)this, #_type, sizeof(*this)); \
    } while (0)

#else
#  define MZ_COUNT_CTOR(_type)
#  define MZ_COUNT_DTOR(_type)
#endif

class LeakDetector {
  std::unique_ptr<QTextStream> m_out = std::make_unique<QTextStream>(stderr);

 public:
  static const QHash<QString, QHash<void*, uint32_t>> getObjects();
  void setOutStream(std::unique_ptr<QTextStream> other);
  LeakDetector();
  ~LeakDetector();

  static void logCtor(void* ptr, const char* typeName, uint32_t size);
  static void logDtor(void* ptr, const char* typeName, uint32_t size);
};

#endif  // LEAKDETECTOR_H
