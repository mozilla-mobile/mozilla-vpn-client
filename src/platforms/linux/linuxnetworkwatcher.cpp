/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "timersingleshot.h"

#include <QtDBus/QtDBus>

// https://developer.gnome.org/NetworkManager/stable/nm-dbus-types.html#NMDeviceType
#ifndef NM_DEVICE_TYPE_WIFI
#  define NM_DEVICE_TYPE_WIFI 2
#endif

// https://developer.gnome.org/NetworkManager/stable/nm-dbus-types.html#NM80211ApFlags
// Wifi network has no security
#ifndef NM_802_11_AP_SEC_NONE
#  define NM_802_11_AP_SEC_NONE 0x00000000
#endif

// Wifi network has WEP (40 bits)
#ifndef NM_802_11_AP_SEC_PAIR_WEP40
#  define NM_802_11_AP_SEC_PAIR_WEP40 0x00000001
#endif

// Wifi network has WEP (104 bits)
#ifndef NM_802_11_AP_SEC_PAIR_WEP104
#  define NM_802_11_AP_SEC_PAIR_WEP104 0x00000002
#endif

constexpr const char* DBUS_NETWORKMANAGER = "org.freedesktop.NetworkManager";

namespace {
Logger logger(LOG_LINUX, "LinuxNetworkWatcher");
}

static inline bool checkSecurityFlags(int securityFlags) {
  return securityFlags == NM_802_11_AP_SEC_NONE ||
         (securityFlags & NM_802_11_AP_SEC_PAIR_WEP40 ||
          securityFlags & NM_802_11_AP_SEC_PAIR_WEP104);
}

LinuxNetworkWatcher::LinuxNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(LinuxNetworkWatcher);
}

LinuxNetworkWatcher::~LinuxNetworkWatcher() {
  MVPN_COUNT_DTOR(LinuxNetworkWatcher);
}

void LinuxNetworkWatcher::initialize() {
  logger.log() << "initialize";

  // Let's wait a few seconds to allow the UI to be fully loaded and shown.
  // This is not strictly needed, but it's better for user experience because
  // it makes the UI faster to appear, plus it gives a bit of delay between the
  // UI to appear and the first notification.
  TimerSingleShot::create(this, 2000, [this]() {
    logger.log()
        << "Retrieving the list of wifi network devices from NetworkManager";

    // To know the NeworkManager DBus methods and properties, read the official
    // documentation:
    // https://developer.gnome.org/NetworkManager/stable/gdbus-org.freedesktop.NetworkManager.html

    QDBusInterface nm(DBUS_NETWORKMANAGER, "/org/freedesktop/NetworkManager",
                      DBUS_NETWORKMANAGER, QDBusConnection::systemBus());
    if (!nm.isValid()) {
      logger.log()
          << "Failed to connect to the network manager via system dbus";
      return;
    }

    QDBusMessage msg = nm.call("GetDevices");
    QDBusArgument arg = msg.arguments().at(0).value<QDBusArgument>();
    if (arg.currentType() != QDBusArgument::ArrayType) {
      logger.log() << "Expected an array of devices";
      return;
    }

    QList<QDBusObjectPath> paths = qdbus_cast<QList<QDBusObjectPath> >(arg);
    for (const QDBusObjectPath& path : paths) {
      QString devicePath = path.path();
      QDBusInterface device(DBUS_NETWORKMANAGER, devicePath,
                            "org.freedesktop.NetworkManager.Device",
                            QDBusConnection::systemBus());
      if (device.property("DeviceType").toInt() != NM_DEVICE_TYPE_WIFI) {
        continue;
      }

      logger.log() << "Found a wifi device:" << devicePath;
      m_devicePaths.append(devicePath);

      // Here we monitor the changes.
      QDBusConnection::systemBus().connect(
          DBUS_NETWORKMANAGER, devicePath, "org.freedesktop.DBus.Properties",
          "PropertiesChanged", this,
          SLOT(propertyChanged(QString, QVariantMap, QStringList)));
    }

    if (m_devicePaths.isEmpty()) {
      logger.log() << "No wifi devices found";
      return;
    }

    // We could be already be activated.
    checkDevices();
  });
}

void LinuxNetworkWatcher::start() {
  logger.log() << "actived";
  NetworkWatcherImpl::start();
  checkDevices();
}

void LinuxNetworkWatcher::propertyChanged(QString interface,
                                          QVariantMap properties,
                                          QStringList list) {
  Q_UNUSED(list);

  logger.log() << "Properties changed for interface" << interface;

  if (!isActive()) {
    logger.log() << "Not active. Ignoring the changes";
    return;
  }

  if (!properties.contains("ActiveAccessPoint")) {
    logger.log() << "Access point did not changed. Ignoring the changes";
    return;
  }

  checkDevices();
}

void LinuxNetworkWatcher::checkDevices() {
  logger.log() << "Checking devices";

  if (!isActive()) {
    logger.log() << "Not active";
    return;
  }

  for (const QString& devicePath : m_devicePaths) {
    QDBusInterface wifiDevice(DBUS_NETWORKMANAGER, devicePath,
                              "org.freedesktop.NetworkManager.Device.Wireless",
                              QDBusConnection::systemBus());

    // Check the access point path
    QString accessPointPath = wifiDevice.property("ActiveAccessPoint")
                                  .value<QDBusObjectPath>()
                                  .path();
    if (accessPointPath.isEmpty()) {
      logger.log() << "No access point found";
      continue;
    }

    QDBusInterface ap(DBUS_NETWORKMANAGER, accessPointPath,
                      "org.freedesktop.NetworkManager.AccessPoint",
                      QDBusConnection::systemBus());

    if (!checkSecurityFlags(ap.property("RsnFlags").toInt()) &&
        !checkSecurityFlags(ap.property("WpaFlags").toInt())) {
      QString ssid = ap.property("Ssid").toString();
      QString bssid = ap.property("HwAddress").toString();

      // We have found 1 unsecured network. We don't need to check other wifi
      // network devices.
      emit unsecuredNetwork(ssid, bssid);
      break;
    }
  }
}
