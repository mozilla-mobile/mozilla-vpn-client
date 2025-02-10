/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <qexception.h>

#include <QFuture>
#include <QHostAddress>
#include <exception>

namespace DNSServerLookup {
constexpr const int MAX_DNS_LOOKUP_ATTEMPTS = 5;
std::optional<QHostAddress> getLocalDNSName();

QFuture<QHostAddress> resolve(const QString& hostname,
                              std::optional<QHostAddress> nameServer);

class DnsFetchException : public QException {
 public:
  DnsFetchException(const QString& msg) : QException() { m_msg = msg; }

  const QString msg() const { return m_msg; }

 private:
  QString m_msg;
};
}  // namespace DNSServerLookup
