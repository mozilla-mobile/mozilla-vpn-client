/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHDATA_H
#define CRASHDATA_H

#include <QObject>
#include <memory>
/**
 * @brief A base class for platform specific crash data sent to the handler.
 * Used to pass through non-platform specific code polymorphically.
 */
class CrashData {
 public:
  CrashData() = default;
  CrashData(const CrashData&) = default;
  CrashData& operator=(const CrashData&) = default;
};
Q_DECLARE_METATYPE(std::shared_ptr<CrashData>);

#endif  // CRASHDATA_H
