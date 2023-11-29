/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOCKDAEMON_H
#define MOCKDAEMON_H

#include "daemon/daemon.h"
#include "wireguardutilsmock.h"

class MockDaemon final : public Daemon {

 public:
  MockDaemon();
  ~MockDaemon();

  static MockDaemon* instance();
  bool activate(const InterfaceConfig& config) override;

 protected:
  WireguardUtils* wgutils() const override { return m_wgutils; }

 private:
  WireguardUtilsMock* m_wgutils = nullptr;
};

#endif  // MOCKDAEMON_H
