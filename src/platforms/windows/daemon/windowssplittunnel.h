/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSSPLITTUNNEL_H
#define WINDOWSSPLITTUNNEL_H

#include <QObject>
#include <QString>
#include <QStringList>

// Note: the ws2tcpip.h import must come before the others.
// clang-format off
#include <ws2tcpip.h>
// clang-format on
#include <Ws2ipdef.h>
#include <ioapiset.h>
#include <tlhelp32.h>
#include <windows.h>

class WindowsSplitTunnel final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsSplitTunnel)
 public:
  explicit WindowsSplitTunnel(QObject* parent);
  ~WindowsSplitTunnel() override;

  // void excludeApps(const QStringList& paths);
  // Excludes an Application from the VPN
  void setRules(const QStringList& appPaths);

  // Fetches and Pushed needed info to move to engaged mode
  void start(int inetAdapterIndex);
  // Deletes Rules and puts the driver into passive mode
  void stop();
  // Resets the Whole Driver
  void reset();

  // Just close connection, leave state as is
  void close();

  // Installes the Kernel Driver as Driver Service
  static SC_HANDLE installDriver();
  static bool uninstallDriver();
  static bool isInstalled();
  static bool detectConflict();

  // States for GetState
  enum DRIVER_STATE {
    STATE_UNKNOWN = -1,
    STATE_NONE = 0,
    STATE_STARTED = 1,
    STATE_INITIALIZED = 2,
    STATE_READY = 3,
    STATE_RUNNING = 4,
    STATE_ZOMBIE = 5,
  };

 private slots:
  void initDriver();

 private:
  HANDLE m_driver = INVALID_HANDLE_VALUE;
  DRIVER_STATE getState();

  // Initializes the WFP Sublayer
  bool initSublayer();

  // Generates a Configuration for Each APP
  std::vector<uint8_t> generateAppConfiguration(const QStringList& appPaths);
  // Generates a Configuration which IP's are VPN and which network
  std::vector<uint8_t> generateIPConfiguration(int inetAdapterIndex);
  std::vector<uint8_t> generateProcessBlob();

  void getAddress(int adapterIndex, IN_ADDR* out_ipv4, IN6_ADDR* out_ipv6);
  // Collects info about an Opened Process

  // Converts a path to a Dos Path:
  // e.g C:/a.exe -> /harddisk0/a.exe
  QString convertPath(const QString& path);
};

#endif  // WINDOWSSPLITTUNNEL_H
