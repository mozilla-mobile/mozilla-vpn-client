/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>

class WindowsDaemon;
class WindowsServiceManager;

namespace Intervention {

class KillerNetwork : public QObject {
  Q_OBJECT
 public:
  /**
   * @brief Returns true if the system is affected
   * can this Intervention can migitate
   */
  static bool systemAffected();

  /**
   * @brief The ID of the intervention for IPC purposes
   *
   * @return const QString&
   */
  static const QString id;

  // Create a new Intervention.
  KillerNetwork(WindowsDaemon* parent);

 private:
  // EventCallbacks for windowsDaemon
  void onVpnActivation();
  void onVpnDeactivation();
  std::unique_ptr<WindowsServiceManager> m_svm;

  bool m_interfered = false;
};

};  // namespace Intervention
