/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QHostAddress>

namespace DNSServerLookup {
std::optional<QHostAddress> getLocalDNSName();

void resolve(const QString& hostname, std::optional<QHostAddress> nameServer,
             QObject* parent);
}  // namespace DNSServerLookup
