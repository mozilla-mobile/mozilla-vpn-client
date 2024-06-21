/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLAVPN_PRIVATE_H
#define MOZILLAVPN_PRIVATE_H

#include "captiveportal/captiveportal.h"
#include "captiveportal/captiveportaldetection.h"
#include "connectionhealth.h"
#include "controller.h"
#include "feature/taskgetfeaturelistworker.h"
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

#if defined MZ_PROXY_ENABLED
#  include "proxycontroller.h"
#endif

struct MozillaVPNPrivate {
  CaptivePortal m_captivePortal;
  CaptivePortalDetection m_captivePortalDetection;
  ConnectionHealth m_connectionHealth;
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
  User m_user;
  TaskGetFeatureListWorker m_taskGetFeatureListWorker;
#if defined MZ_PROXY_ENABLED
  ProxyController m_proxyController;
#endif
};

#endif  // MOZILLAVPN_PRIVATE_H
