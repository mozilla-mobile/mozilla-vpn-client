/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXBYPASS_H
#define LINUXBYPASS_H

class QAbstractSocket;
class QHostAddress;

void setupLinuxBypass(QAbstractSocket* s, const QHostAddress& dest);

#endif  // LINUXBYPASS_H
