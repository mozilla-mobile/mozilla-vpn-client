/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5connection.h"

// static
QString Socks5Connection::localClientName(QLocalSocket* s) {
  Q_UNUSED(s);
  return QString();
}
