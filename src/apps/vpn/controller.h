/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "connectionmanager.h"
#include "loghandler.h"

class ControllerImpl;
class MozillaVPN;

class Controller final : public QObject, public LogSerializer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Controller)

 public:
  Controller();
  ~Controller();
  
  void initialize();

  bool isVPNActive();

  // LogSerializer interface
  void serializeLogs(
      std::function<void(const QString& name, const QString& logs)>&& callback)
      override;

 public slots:
  // These 2 methods activate/deactivate the VPN. Return true if a signal will
  // be emitted at the end of the operation.
//  bool activate(
//      const ServerData& serverData,
//      ServerSelectionPolicy serverSelectionPolicy = RandomizeServerSelection);
  bool deactivate();

 private:
  bool m_VPNActive = false;
  QScopedPointer<ControllerImpl> m_impl;
  bool m_initialized = false;
};

#endif  // CONTROLLER_H
