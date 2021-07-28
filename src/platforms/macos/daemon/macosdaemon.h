/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDAEMON_H
#define MACOSDAEMON_H

#include "daemon.h"
#include "iputilsmacos.h"
#include "wireguardutilsmacos.h"

class MacOSDaemon final : public Daemon {
  friend class IPUtilsMacos;

 public:
  MacOSDaemon();
  ~MacOSDaemon();

  static MacOSDaemon* instance();

  QByteArray getStatus() override;

  void maybeCleanup();

 protected:
  bool run(Op op, const InterfaceConfig& config) override;
  bool supportWGUtils() const override { return true; }
  WireguardUtils* wgutils() override { return m_wgutils; }
  bool supportIPUtils() const override { return true; }
  IPUtils* iputils() override { return m_iputils; }

 private:
  WireguardUtilsMacos* m_wgutils = nullptr;
  IPUtilsMacos* m_iputils = nullptr;
};

#endif  // MACOSDAEMON_H
