/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "timersingleshot.h"

#include <QMap>
#include <QVariant>
#include <QtDBus/QtDBus>

namespace {
Logger logger(LOG_LINUX, "LinuxNetworkWatcher");
}

LinuxNetworkWatcher::LinuxNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(LinuxNetworkWatcher);
}

LinuxNetworkWatcher::~LinuxNetworkWatcher() {
  MVPN_COUNT_DTOR(LinuxNetworkWatcher);
}

void LinuxNetworkWatcher::initialize() { logger.log() << "initialize"; }

void LinuxNetworkWatcher::start() {
  logger.log() << "actived";
  m_active = true;

  TimerSingleShot::create(this, 2000, [this]() {
    QDBusInterface nm(
        "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    if (!nm.isValid()) {
      logger.log() << "Failed to connect to system bus";
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
      QDBusInterface device("org.freedesktop.NetworkManager", devicePath,
                            "org.freedesktop.NetworkManager.Device",
                            QDBusConnection::systemBus());
      // 2 is WiFi dev, see
      // https://people.freedesktop.org/~lkundrak/nm-docs/nm-dbus-types.html#NMDeviceType
      if (device.property("DeviceType").toInt() != 2) {
        continue;
      }

      // we got a wifi device, let's get an according dbus interface
      QDBusInterface wifiDevice(
          "org.freedesktop.NetworkManager", devicePath,
          "org.freedesktop.NetworkManager.Device.Wireless",
          QDBusConnection::systemBus());

      wifiDevice.connection().connect(
          "org.freedesktop.NetworkManager", devicePath,
          "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
          SLOT(propertyChanged(QString, QVariantMap, QStringList)));

      m_devicePaths.append(devicePath);
    }
  });

  checkDevices();
}

void LinuxNetworkWatcher::stop() {
  logger.log() << "deactived";
  m_active = false;
}

void LinuxNetworkWatcher::propertyChanged(QString interface,
                                          QVariantMap properties,
                                          QStringList list) {
  Q_UNUSED(list);

  logger.log() << "Properties changed for interface" << interface;

  if (!m_active) {
    logger.log() << "Not active";
    return;
  }

  if (!properties.contains("ActiveAccessPoint")) {
    logger.log() << "Access point did not changed";
    return;
  }

  checkDevices();
}

void LinuxNetworkWatcher::checkDevices() {
  logger.log() << "Checking devices";

  for (const QString& devicePath : m_devicePaths) {
    QDBusInterface wifiDevice("org.freedesktop.NetworkManager", devicePath,
                              "org.freedesktop.NetworkManager.Device.Wireless",
                              QDBusConnection::systemBus());

    // Check the access point path
    QString accessPointPath = wifiDevice.property("ActiveAccessPoint")
                                  .value<QDBusObjectPath>()
                                  .path();
    if (accessPointPath.isEmpty()) {
      logger.log() << "No access point";
      continue;
    }

    QDBusInterface ap("org.freedesktop.NetworkManager", accessPointPath,
                      "org.freedesktop.NetworkManager.AccessPoint",
                      QDBusConnection::systemBus());

    // https://developer.gnome.org/NetworkManager/stable/nm-dbus-types.html#NM80211ApFlags
    int securityFlags = ap.property("WpaFlags").toInt();

#ifndef NM_802_11_AP_SEC_NONE
#  define NM_802_11_AP_SEC_NONE 0x00000000
#endif

#ifndef NM_802_11_AP_SEC_PAIR_WEP40
#  define NM_802_11_AP_SEC_PAIR_WEP40 0x00000001
#endif

#ifndef NM_802_11_AP_SEC_PAIR_WEP104
#  define NM_802_11_AP_SEC_PAIR_WEP104 0x00000001
#endif

    if (securityFlags == NM_802_11_AP_SEC_NONE ||
        (securityFlags & NM_802_11_AP_SEC_PAIR_WEP40 ||
         securityFlags & NM_802_11_AP_SEC_PAIR_WEP104)) {
      QString ssid = ap.property("Ssid").toString();
      QString bssid = ap.property("HwAddress").toString();

      emit unsecuredNetwork(ssid, bssid);
    }
  }
}
