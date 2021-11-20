/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSROUTEMONITOR_H
#define WINDOWSROUTEMONITOR_H

#include <QHostAddress>
#include <QObject>

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

class WindowsRouteMonitor final : public QObject {
  Q_OBJECT

 public:
  WindowsRouteMonitor(QObject* parent);
  ~WindowsRouteMonitor();

  bool addExclusionRoute(const QHostAddress& address);
  bool addExclusionRoute(const QString& address) { 
    return addExclusionRoute(QHostAddress(address));
  }

  void deleteExclusionRoute(const QHostAddress& address);
  void deleteExclusionRoute(const QString& address) {
    deleteExclusionRoute(QHostAddress(address));
  }

  void flushExclusionRoutes();

  void setLuid(quint64 luid) { m_luid = luid; }
  quint64 getLuid() { return m_luid; }

 public slots:
  void routeChanged();

 private:
  void updateExclusionRoute(MIB_IPFORWARD_ROW2* data, void* table);

  QHash<QHostAddress, MIB_IPFORWARD_ROW2*> m_exclusionRoutes;

  quint64 m_luid = 0;
  HANDLE m_routeHandle = INVALID_HANDLE_VALUE;
};

#endif /* WINDOWSROUTEMONITOR_H */
