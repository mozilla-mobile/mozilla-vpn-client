/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IPADDRESSLOOKUP_H
#define IPADDRESSLOOKUP_H

#include <QObject>
#include <QTimer>

#ifdef UNIT_TEST
class TestIpAddressLookup;
#endif

class IpAddressLookup final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IpAddressLookup)

  Q_PROPERTY(QString ipv4Address READ ipv4Address NOTIFY ipv4AddressChanged)
  Q_PROPERTY(QString ipv6Address READ ipv6Address NOTIFY ipv6AddressChanged)

 public:
  IpAddressLookup();
  ~IpAddressLookup();

  void initialize();

  const QString& ipv4Address() const { return m_ipv4Address; }
  const QString& ipv6Address() const { return m_ipv6Address; }

 private:
  void updateIpAddress();

  void reset();

  void stateChanged();

 signals:
  // for testing.
  void ipAddressChecked();

  void ipv4AddressChanged();
  void ipv6AddressChanged();

 private:
  enum {
    // Waiting for the VPN to turn on.
    StateWaiting,

    // Refreshing the IP address.
    StateUpdating,

    // IP address is a known.
    StateUpdated,
  }  // Let's use `StateUpdated` to force a refresh in the CTOR.
  m_state = StateUpdated;

  QString m_ipv4Address;
  QString m_ipv6Address;
  QTimer m_ipAddressTimer;

#ifdef UNIT_TEST
  friend class TestIpAddressLookup;
#endif
};

#endif  // IPADDRESSLOOKUP_H
