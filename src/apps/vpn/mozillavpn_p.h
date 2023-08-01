/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLAVPN_PRIVATE_H
#define MOZILLAVPN_PRIVATE_H

#include "captiveportal/captiveportal.h"
#include "captiveportal/captiveportaldetection.h"
#include "connectionbenchmark/connectionbenchmark.h"
#include "connectionhealth.h"
#include "controller.h"
#include "connectionmanager.h"
#include "ipaddresslookup.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/location.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "models/subscriptiondata.h"
#include "models/supportcategorymodel.h"
#include "models/user.h"
#include "networkwatcher.h"
#include "profileflow.h"
#include "releasemonitor.h"
#include "serverlatency.h"
#include "statusicon.h"
#include "telemetry.h"
#include "websocket/websockethandler.h"

struct MozillaVPNPrivate {
  CaptivePortal m_captivePortal;
  CaptivePortalDetection m_captivePortalDetection;
  ConnectionBenchmark m_connectionBenchmark;
  ConnectionHealth m_connectionHealth;
  ConnectionManager m_connectionManager;
  Controller m_controller;
  DeviceModel m_deviceModel;
  IpAddressLookup m_ipAddressLookup;
  Keys m_keys;
  Location m_location;
  NetworkWatcher m_networkWatcher;
  ReleaseMonitor m_releaseMonitor;
  ServerCountryModel m_serverCountryModel;
  ServerData m_serverData;
  ServerLatency m_serverLatency;
  StatusIcon m_statusIcon;
  SubscriptionData m_subscriptionData;
  SupportCategoryModel m_supportCategoryModel;
  ProfileFlow m_profileFlow;
  Telemetry m_telemetry;
  WebSocketHandler m_webSocketHandler;
  User m_user;
};

#endif  // MOZILLAVPN_PRIVATE_H
