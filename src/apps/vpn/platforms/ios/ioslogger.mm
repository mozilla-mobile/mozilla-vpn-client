/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioslogger.h"
#include "Mozilla-Swift.h"
#include "leakdetector.h"

#include <QString>

namespace {

// Our Swift singleton.
IOSLoggerImpl* impl = nullptr;

}  // namespace

IOSLogger::IOSLogger(const QString& tag) {
  MZ_COUNT_CTOR(IOSLogger);

  impl = [[IOSLoggerImpl alloc] initWithTag:tag.toNSString()];
  Q_ASSERT(impl);
}

IOSLogger::~IOSLogger() {
  MZ_COUNT_DTOR(IOSLogger);

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}

// static
void IOSLogger::debug(const QString& message) {
    [impl debugWithMessage:message.toNSString()];
}

// static
void IOSLogger::info(const QString& message) { [impl infoWithMessage:message.toNSString()]; }

// static
void IOSLogger::error(const QString& message) { [impl errorWithMessage:message.toNSString()]; }
