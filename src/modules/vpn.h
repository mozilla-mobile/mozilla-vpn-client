/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULEVPN_H
#define MODULEVPN_H

#include "module.h"
#include "modules/vpn/captiveportal/captiveportal.h"
#include "modules/vpn/captiveportal/captiveportaldetection.h"
#include "modules/vpn/connectionbenchmark/connectionbenchmark.h"
#include "modules/vpn/connectionhealth.h"
#include "modules/vpn/controller.h"
#include "modules/vpn/dnshelper.h"
#include "modules/vpn/ipaddresslookup.h"
#include "modules/vpn/keyregenerator.h"
#include "modules/vpn/networkwatcher.h"
#include "modules/vpn/serverlatency.h"
#include "modules/vpn/telemetry.h"

#include <QJSValue>

class ModuleVPN final : public Module {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ModuleVPN)

  Q_PROPERTY(
      QJSValue captivePortalDetection READ captivePortalDetectionValue CONSTANT)
  Q_PROPERTY(
      QJSValue connectionBenchmark READ connectionBenchmarkValue CONSTANT)
  Q_PROPERTY(QJSValue connectionHealth READ connectionHealthValue CONSTANT)
  Q_PROPERTY(QJSValue controller READ controllerValue CONSTANT)
  Q_PROPERTY(QJSValue ipAddressLookup READ ipAddressLookupValue CONSTANT)

 public:
  explicit ModuleVPN(QObject* parent);
  ~ModuleVPN();

  void initialize() override;

  void settingsAvailable() override;

  void updateRequired() override;

  void quit() override;

  void backendFailure() override;

  void serializeLogs(QTextStream* out,
                     std::function<void()>&& a_finalizeCallback) override;

  void cleanupLogs() override;

  static ModuleVPN* instance();

  /**
   * @brief activate the VPN
   */
  Q_INVOKABLE void activate();

  /**
   * @brief deactivate the VPN
   */
  Q_INVOKABLE void deactivate();

  /**
   * @brief Validate a user-input DNS entry
   */
  Q_INVOKABLE bool validateUserDNS(const QString& dns) const;

  /**
   * @brief Getter for the CaptivePortal object
   */
  CaptivePortal* captivePortal() { return &m_captivePortal; }

  /**
   * @brief Getter for the CaptivePortalDetection
   */
  CaptivePortalDetection* captivePortalDetection() {
    return &m_captivePortalDetection;
  }

  /**
   * @brief Getter for the ConnectionBenchmark object
   */
  ConnectionBenchmark* connectionBenchmark() { return &m_connectionBenchmark; }

  /**
   * @brief Getter for the ConnectionHealth object
   */
  ConnectionHealth* connectionHealth() { return &m_connectionHealth; }

  /**
   * @brief Getter for the VPN Controller object
   */
  Controller* controller() { return &m_controller; }

  /**
   * @brief getter for the IpAddressLookup
   */
  IpAddressLookup* ipAddressLookup() { return &m_ipAddressLookup; }

  /**
   * @brief getter for the NetworkWatcher object
   */
  NetworkWatcher* networkWatcher() { return &m_networkWatcher; }

  void silentSwitch();

 private:
  QJSValue captivePortalDetectionValue();
  QJSValue connectionBenchmarkValue();
  QJSValue connectionHealthValue();
  QJSValue controllerValue();
  QJSValue ipAddressLookupValue();

  void controllerStateChanged();

  void registerInspectorHandlerCommands();
  void registerServerConnectionRequestTypes();
  void registerTutorialStepEmitters();

  void maybeShowNotification();

  void serverConnectionStateUpdate();

 private:
  CaptivePortal m_captivePortal;
  CaptivePortalDetection m_captivePortalDetection;
  ConnectionBenchmark m_connectionBenchmark;
  ConnectionHealth m_connectionHealth;
  Controller m_controller;
  IpAddressLookup m_ipAddressLookup;
  KeyRegenerator m_keyRegenerator;
  NetworkWatcher m_networkWatcher;
  ServerLatency m_serverLatency;
  Telemetry m_telemetry;

  bool m_controllerInitialized = false;

  // We want to show a 'disconnected' notification only if we were actually
  // connected.
  bool m_connected = false;
  bool m_switching = false;
};

#endif  // MODULEVPN_H
