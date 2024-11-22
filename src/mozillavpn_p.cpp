/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn_p.h"

#include "mozillavpn.h"

CaptivePortal* MozillaVPN::captivePortal() const {
  return &m_private->m_captivePortal;
}
CaptivePortalDetection* MozillaVPN::captivePortalDetection() const {
  return &m_private->m_captivePortalDetection;
}

ConnectionHealth* MozillaVPN::connectionHealth() const {
  return &m_private->m_connectionHealth;
}

Controller* MozillaVPN::controller() const { return &m_private->m_controller; }

DeviceModel* MozillaVPN::deviceModel() const {
  return &m_private->m_deviceModel;
}

IpAddressLookup* MozillaVPN::ipAddressLookup() const {
  return &m_private->m_ipAddressLookup;
}

Keys* MozillaVPN::keys() const { return &m_private->m_keys; }

Location* MozillaVPN::location() const { return &m_private->m_location; }

NetworkWatcher* MozillaVPN::networkWatcher() const {
  return &m_private->m_networkWatcher;
}

ProfileFlow* MozillaVPN::profileFlow() const {
  return &m_private->m_profileFlow;
}

ReleaseMonitor* MozillaVPN::releaseMonitor() const {
  return &m_private->m_releaseMonitor;
}

ServerData* MozillaVPN::serverData() const { return &m_private->m_serverData; }

ServerCountryModel* MozillaVPN::serverCountryModel() const {
  return &m_private->m_serverCountryModel;
}

ServerLatency* MozillaVPN::serverLatency() const {
  return &m_private->m_serverLatency;
}

StatusIcon* MozillaVPN::statusIcon() const { return &m_private->m_statusIcon; }

SubscriptionData* MozillaVPN::subscriptionData() const {
  return &m_private->m_subscriptionData;
}

SupportCategoryModel* MozillaVPN::supportCategoryModel() const {
  return &m_private->m_supportCategoryModel;
}

Telemetry* MozillaVPN::telemetry() const { return &m_private->m_telemetry; }

User* MozillaVPN::user() const { return &m_private->m_user; }
