/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDLOGGER_H
#define ANDROIDLOGGER_H

#include <QObject>

namespace AndroidLogger {

// Creates a "share" intent to Open/Send Plaintext
bool shareText(const QString& plainText);
};  // namespace AndroidLogger

#endif  // ANDROIDLOGGER_H
