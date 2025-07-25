/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BUILDINFO_H
#define BUILDINFO_H

#include <QString>

namespace BuildInfo {

extern const QString version;
extern const qint64 timestamp;
extern const QString commit;
extern const QString number;

};  // namespace BuildInfo

#endif  // BUILDINFO_H
