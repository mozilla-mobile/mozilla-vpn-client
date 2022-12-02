/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULEVPN_H
#define MODULEVPN_H

#include "module.h"
#include "modules/modulevpn/captiveportal/captiveportal.h"
#include "modules/modulevpn/captiveportal/captiveportaldetection.h"
#include "modules/modulevpn/connectionbenchmark/connectionbenchmark.h"
#include "modules/modulevpn/connectionhealth.h"
#include "modules/modulevpn/controller.h"

#include <QJSValue>

class ModuleVPN final : public Module {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ModuleVPN)

  Q_PROPERTY(
      QJSValue connectionBenchmark READ connectionBenchmarkValue CONSTANT)
  Q_PROPERTY(QJSValue connectionHealth READ connectionHealthValue CONSTANT)
  Q_PROPERTY(QJSValue controller READ controllerValue CONSTANT)

 public:
  explicit ModuleVPN(QObject* parent);
  ~ModuleVPN();

  static ModuleVPN* instance();

  CaptivePortal* captivePortal() { return &m_captivePortal; }

  CaptivePortalDetection* captivePortalDetection() {
    return &m_captivePortalDetection;
  }
  QJSValue captivePortalDetectionValue();

  ConnectionBenchmark* connectionBenchmark() { return &m_connectionBenchmark; }
  QJSValue connectionBenchmarkValue();

  ConnectionHealth* connectionHealth() { return &m_connectionHealth; }
  QJSValue connectionHealthValue();

  Controller* controller() { return &m_controller; }
  QJSValue controllerValue();

 private:
  CaptivePortal m_captivePortal;
  CaptivePortalDetection m_captivePortalDetection;
  ConnectionBenchmark m_connectionBenchmark;
  ConnectionHealth m_connectionHealth;
  Controller m_controller;
};

#endif  // MODULEVPN_H
