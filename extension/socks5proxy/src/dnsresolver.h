/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QGlobalStatic>
#include <QHostAddress>
#include <mutex>

struct ares_channeldata;
class ares_addrinfo;
class Socks5Connection;

class DNSResolver {
 public:
  DNSResolver();
  ~DNSResolver();

  static DNSResolver* instance();

  /**
   * @brief Queues up a DNS Query to get Resolved.
   *
   * @param hostname - The requested Hostname
   * @param nameServer - Optional sets the DNS server
   * @param parent - The Socks5Connection to notify. Will call
   * Socks5Connection::onHostnameResolved(QHostAddress) when done.
   */
  void resolveAsync(const QString& hostname,
                    Socks5Connection* parent);

  void setNameserver(const QHostAddress& addr);

 private:
  static void addressInfoCallback(void* arg, int status, int timeouts,
                                  struct ares_addrinfo* result);
  void shutdownAres();

  ares_channeldata* mChannel = nullptr;

  QHostAddress m_nameserver;
};
