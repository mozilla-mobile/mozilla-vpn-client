/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "killernetwork.h"

#include <qobject.h>

#include <QObject>

#include "daemon/daemon.h"
#include "platforms/windows/daemon/windowsdaemon.h"
#include "platforms/windows/windowsservicemanager.h"

namespace Intervention {

const QString KillerNetwork::id = "intel.killernetwork";

bool KillerNetwork::systemAffected() {
  std::unique_ptr<WindowsServiceManager> svm =
      WindowsServiceManager::open("TODONETWORKSERVER");
  return svm == nullptr;
}

KillerNetwork::KillerNetwork(WindowsDaemon* aParent) : QObject(aParent) {
  if (!systemAffected()) {
    deleteLater();
    return;
  }
  m_svm = WindowsServiceManager::open("TODONETWORKSERVER");
  if (!m_svm) {
    deleteLater();
    return;
  }
  connect(aParent, &Daemon::connected, this, &KillerNetwork::onVpnActivation);
  connect(aParent, &Daemon::disconnected, this,
          &KillerNetwork::onVpnDeactivation);
}

void KillerNetwork::onVpnActivation() {
  if (m_svm->stopService()) {
    m_interfered = true;
  }
}
void KillerNetwork::onVpnDeactivation() {
  if (!m_interfered) {
    return;
  }
  m_svm->startService();
  m_interfered = false;
}

}  // namespace Intervention
