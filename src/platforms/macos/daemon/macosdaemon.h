/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDAEMON_H
#define MACOSDAEMON_H

#include "daemon/daemon.h"

class MacOSDaemon final : public Daemon {
  friend class IPUtilsMacos;

 public:
  MacOSDaemon(QObject* parent = nullptr);
  ~MacOSDaemon();

  static MacOSDaemon* instance();

 protected:
  WireguardUtils* wgutils() const override { return m_wgutils; }
  DnsUtils* dnsutils() override { return m_dnsutils; }
  IPUtils* iputils() override { return m_iputils; }
  FirewallUtils* fwutils() override { return m_fwutils; }

 private:
  WireguardUtils* m_wgutils = nullptr;
  DnsUtils* m_dnsutils = nullptr;
  IPUtils* m_iputils = nullptr;
  FirewallUtils* m_fwutils = nullptr;
};

#endif  // MACOSDAEMON_H
