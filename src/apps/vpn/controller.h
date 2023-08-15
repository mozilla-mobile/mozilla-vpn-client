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

  bool isVPNActive();

  // LogSerializer interface
  void serializeLogs(
      std::function<void(const QString& name, const QString& logs)>&& callback)
      override;

 public slots:

 private:
  bool m_VPNActive = false;
  QScopedPointer<ControllerImpl> m_impl;
};

#endif  // CONTROLLER_H
