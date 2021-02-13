/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkwatcher.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcherimpl.h"
#include "settingsholder.h"
#include "systemtrayhandler.h"

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowsnetworkwatcher.h"
#endif

#ifdef MVPN_LINUX
#  include "platforms/linux/linuxnetworkwatcher.h"
#endif

// How often we notify the same unsecured network
constexpr uint32_t NETWORK_WATCHER_TIMER_MSEC = 20000;

namespace {
Logger logger(LOG_NETWORKING, "NetworkWatcher");
}

NetworkWatcher::NetworkWatcher() {
  MVPN_COUNT_CTOR(NetworkWatcher);

  m_notifyTimer.setSingleShot(true);
}

NetworkWatcher::~NetworkWatcher() { MVPN_COUNT_DTOR(NetworkWatcher); }

void NetworkWatcher::initialize() {
  logger.log() << "Initialize";

#if defined(MVPN_WINDOWS)
  m_impl = new WindowsNetworkWatcher(this);
#elif defined(MVPN_LINUX)
  m_impl = new LinuxNetworkWatcher(this);
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

#ifndef UNIT_TEST
  if (!m_active) {
    logger.log() << "Disabled. Ignoring unsecured network";
    return;
  }

  Controller::State state = MozillaVPN::instance()->controller()->state();
  if (state == Controller::StateOn || state == Controller::StateConnecting ||
      state == Controller::StateSwitching) {
    logger.log() << "VPN on. Ignoring unsecured network";
    return;
  }

  if (!m_networks.contains(networkId)) {
    m_networks.insert(networkId, QElapsedTimer());
  } else if (!m_networks[networkId].hasExpired(NETWORK_WATCHER_TIMER_MSEC)) {
    logger.log() << "Notification already shown. Ignoring unsecured network";
    return;
  }

  // Let's activate the QElapsedTimer to avoid notification loops.
  m_networks[networkId].start();

  // We don't connect the system tray handler in the CTOR because it can be too
  // early. Maybe the SystemTrayHandler has not been created yet. We do it at
  // the first detection of an unsecured network.
  if (m_firstNotification) {
    connect(SystemTrayHandler::instance(), &QSystemTrayIcon::messageClicked,
            this, &NetworkWatcher::messageClicked);
    m_firstNotification = false;
  }

  m_notifyTimer.start(Constants::UNSECURED_NETWORK_ALERT_MSEC);
  SystemTrayHandler::instance()->unsecuredNetworkNotification(networkName);
#endif
}

void NetworkWatcher::messageClicked() {
  logger.log() << "Message clicked";

  if (!m_notifyTimer.isActive()) {
    logger.log() << "The message is not for us. Let's ignore it.";
    return;
  }

  MozillaVPN::instance()->activate();
}
