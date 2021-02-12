/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcherimpl.h"
#include "settingsholder.h"
#include "systemtrayhandler.h"

#ifdef MVPN_WINDOWS
#include "platforms/windows/windowsnetworkwatcher.h"
#endif

// How often we notify the same unsecured network
constexpr uint32_t NETWORK_WATCHER_TIMER_MSEC = 20000;

namespace {
Logger logger(LOG_NETWORKING, "NetworkWatcher");
}

NetworkWatcher::NetworkWatcher() { MVPN_COUNT_CTOR(NetworkWatcher); }

NetworkWatcher::~NetworkWatcher() { MVPN_COUNT_DTOR(NetworkWatcher); }

void NetworkWatcher::initialize() {
  logger.log() << "Initialize";

#ifdef MVPN_WINDOWS
  m_impl = new WindowsNetworkWatcher(this);
#else
  logger.log()
      << "No NetworkWatcher implementation for the current platform (yet)";
  return;
#endif

  connect(m_impl, &NetworkWatcherImpl::unsecuredNetwork, this,
          &NetworkWatcher::unsecuredNetwork);

  m_impl->initialize();

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  m_active = settingsHolder->unsecuredNetworkAlert();
  if (m_active) {
    m_impl->start();
  }

  connect(settingsHolder, &SettingsHolder::unsecuredNetworkAlertChanged, this,
          &NetworkWatcher::settingsChanged);
}

void NetworkWatcher::settingsChanged(bool active) {
  logger.log() << "Settings changed:" << active;
  if (m_active == active) {
    return;
  }

  m_active = active;

  if (!m_impl) {
    return;
  }

  if (m_active) {
    m_impl->start();
  } else {
    m_impl->stop();
  }
}

void NetworkWatcher::unsecuredNetwork(const QString& networkName,
                                      const QString& networkId) {
  logger.log() << "Unsecured network:" << networkName << "id:" << networkId;

  if (!m_active) {
    logger.log() << "Disabled. Ignore unsecured network";
    return;
  }

  Controller::State state = MozillaVPN::instance()->controller()->state();
  if (state == Controller::StateOn || state == Controller::StateConnecting || state == Controller::StateSwitching) {
    logger.log() << "VPN on. Ignore unsecured network";
    return;
  }

  if (!m_networks.contains(networkId)) {
    m_networks.insert(networkId, QElapsedTimer());
  } else if (!m_networks[networkId].hasExpired(NETWORK_WATCHER_TIMER_MSEC)) {
    return;
  }

  m_networks[networkId].start();

  SystemTrayHandler::instance()->unsecuredNetworkNotification(networkName);
}
