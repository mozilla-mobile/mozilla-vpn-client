/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACFWPOLICY_H
#define MACFWPOLICY_H

#include <QObject>

class Socks5;
class ProxyConnection;

class MacFwPolicy final : public QObject {
  Q_OBJECT

 public:
  explicit MacFwPolicy(Socks5* proxy);
  ~MacFwPolicy() = default;

 private:
  void checkLocalSocket(ProxyConnection* connection);

  // The signing identifiers of known web browsers.
  QStringList m_browsers;
};

#endif  // MACFWPOLICY_H
