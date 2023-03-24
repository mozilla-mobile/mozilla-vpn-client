/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidcontroller.h"

#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "androidutils.h"
#include "androidvpnactivity.h"
#include "errorhandler.h"
#include "i18nstrings.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "settingsholder.h"

namespace {
Logger logger("AndroidController");
AndroidController* s_instance = nullptr;

}  // namespace

AndroidController::AndroidController() {
  MZ_COUNT_CTOR(AndroidController);
  s_instance = this;

  auto activity = AndroidVPNActivity::instance();

  connect(
      activity, &AndroidVPNActivity::serviceConnected, this,
      []() {
        AndroidVPNActivity::sendToService(ServiceAction::ACTION_GET_STATUS, "");
      },
      Qt::QueuedConnection);
  connect(
      activity, &AndroidVPNActivity::eventInitialized, this,
      [this](const QString& parcelBody) {
        // We might get multiple Init events as widgets, or fragments
        // might query this.
        if (m_init) {
          return;
        }
        m_init = true;
        auto doc = QJsonDocument::fromJson(parcelBody.toUtf8());
        qlonglong time = doc.object()["time"].toVariant().toLongLong();
        emit initialized(
            true, doc.object()["connected"].toBool(),
            time > 0 ? QDateTime::fromMSecsSinceEpoch(time) : QDateTime());
      },
      Qt::QueuedConnection);

  connect(
      activity, &AndroidVPNActivity::eventConnected, this,
      [this](const QString& parcelBody) {
        auto doc = QJsonDocument::fromJson(parcelBody.toUtf8());
        qlonglong time = doc.object()["time"].toVariant().toLongLong();
        Q_UNUSED(parcelBody);
        emit connected(
            m_serverPublicKey,
            time > 0 ? QDateTime::fromMSecsSinceEpoch(time) : QDateTime());
      },
      Qt::QueuedConnection);
  connect(activity, &AndroidVPNActivity::eventDisconnected, this,
          &AndroidController::disconnected, Qt::QueuedConnection);
  connect(
      activity, &AndroidVPNActivity::eventStatisticUpdate, this,
      [this](const QString& parcelBody) {
        auto doc = QJsonDocument::fromJson(parcelBody.toUtf8());
        emit statusUpdated(doc.object()["endpoint"].toString(),
                           doc.object()["deviceIpv4"].toString(),
                           doc.object()["tx_bytes"].toInt(),
                           doc.object()["rx_bytes"].toInt());
      },
      Qt::QueuedConnection);
  connect(
      activity, &AndroidVPNActivity::eventActivationError, this,
      [this](const QString& parcelBody) {
        if (!parcelBody.isEmpty()) {
          logger.error() << "Service Error while activating the VPN: "
                         << parcelBody;
        }
        REPORTERROR(ErrorHandler::ConnectionFailureError, "controller");
        emit disconnected();
      },
      Qt::QueuedConnection);
  connect(
      activity, &AndroidVPNActivity::serviceDisconnected, this,
      []() { REPORTERROR(ErrorHandler::ControllerError, "controller"); },
      Qt::QueuedConnection);
}
AndroidController::~AndroidController() { MZ_COUNT_DTOR(AndroidController); }

void AndroidController::initialize(const Device* device, const Keys* keys) {
  logger.debug() << "Initializing";

  Q_UNUSED(device);
  Q_UNUSED(keys);

  AndroidVPNActivity::connectService();
}

void AndroidController::activate(const HopConnection& hop, const Device* device,
                                 const Keys* keys, Controller::Reason reason) {
  Q_ASSERT(hop.m_hopindex == 0);
  logger.debug() << "Activation";

  m_device = *device;
  m_serverPublicKey = hop.m_server.publicKey();

  // Serialise arguments for the VPNService
  QJsonObject jDevice;
  jDevice["publicKey"] = device->publicKey();
  jDevice["name"] = device->name();
  jDevice["createdAt"] = device->createdAt().toMSecsSinceEpoch();
  jDevice["ipv4Address"] = device->ipv4Address();
  jDevice["ipv6Address"] = device->ipv6Address();

  QJsonObject jKeys;
  jKeys["privateKey"] = keys->privateKey();

  QJsonObject jServer;
  logger.info() << "Server" << logger.sensitive(hop.m_server.hostname());
  jServer["ipv4AddrIn"] = hop.m_server.ipv4AddrIn();
  jServer["ipv4Gateway"] = hop.m_server.ipv4Gateway();
  jServer["ipv6AddrIn"] = hop.m_server.ipv6AddrIn();
  jServer["ipv6Gateway"] = hop.m_server.ipv6Gateway();

  jServer["publicKey"] = hop.m_server.publicKey();
  jServer["port"] = (double)hop.m_server.choosePort();

  QList<IPAddress> allowedIPs;
  QList<IPAddress> excludedIPs;
  QJsonArray fullAllowedIPs;
  foreach (auto item, hop.m_allowedIPAddressRanges) {
    allowedIPs.append(IPAddress(item.toString()));
  }
  foreach (auto addr, hop.m_excludedAddresses) {
    excludedIPs.append(IPAddress(addr));
  }
  foreach (auto item, IPAddress::excludeAddresses(allowedIPs, excludedIPs)) {
    fullAllowedIPs.append(QJsonValue(item.toString()));
  }

  QJsonArray excludedApps;
  foreach (auto appID, hop.m_vpnDisabledApps) {
    excludedApps.append(QJsonValue(appID));
  }

  // Find a Server as Fallback in the Same Location in case
  // the original one becomes unstable / unavailable
  auto vpn = MozillaVPN::instance();
  const QList<Server> serverList =
      vpn->controller()->currentServer().exitServers();
  Server* fallbackServer = nullptr;
  foreach (auto item, serverList) {
    if (item.publicKey() != hop.m_server.publicKey()) {
      fallbackServer = &item;
      break;
    }
  }
  QJsonObject jFallbackServer;
  if (fallbackServer) {
    jFallbackServer["ipv4AddrIn"] = fallbackServer->ipv4AddrIn();
    jFallbackServer["ipv4Gateway"] = fallbackServer->ipv4Gateway();
    jFallbackServer["ipv6AddrIn"] = fallbackServer->ipv6AddrIn();
    jFallbackServer["ipv6Gateway"] = fallbackServer->ipv6Gateway();

    jFallbackServer["publicKey"] = fallbackServer->publicKey();
    jFallbackServer["port"] = (double)fallbackServer->choosePort();
  }

  QJsonObject args;
  args["device"] = jDevice;
  args["keys"] = jKeys;
  args["server"] = jServer;
  args["reason"] = (int)reason;
  args["allowedIPs"] = fullAllowedIPs;
  args["excludedApps"] = excludedApps;
  args["dns"] = hop.m_dnsServer.toString();
  if (fallbackServer) {
    args["serverFallback"] = jFallbackServer;
  }
  // Build the "canned" Notification messages
  // They will be used in case this config will be re-enabled
  // to show the appropriate notification messages
  QString localizedCityName =
      vpn->controller()->currentServer().localizedExitCityName();
  args["city"] = localizedCityName;

  QJsonObject messages;
  messages["productName"] = I18nStrings::instance()->t(
      I18nStrings::ProductName);
  messages["connectedHeader"] = I18nStrings::instance()->t(
      I18nStrings::NotificationsVPNConnectedTitle);  // Connected
  messages["connectedBody"] =
      I18nStrings::instance()
          ->t(I18nStrings::NotificationsVPNConnectedMessage)
          .arg(localizedCityName);
  messages["disconnectedHeader"] = I18nStrings::instance()->t(
      I18nStrings::NotificationsVPNDisconnectedTitle);
  messages["disconnectedBody"] =
      I18nStrings::instance()
          ->t(I18nStrings::NotificationsVPNDisconnectedMessage)
          .arg(localizedCityName);
  args["messages"] = messages;

  QJsonDocument doc(args);
  AndroidVPNActivity::sendToService(ServiceAction::ACTION_ACTIVATE,
                                    doc.toJson(QJsonDocument::Compact));
}

void AndroidController::deactivate(Controller::Reason reason) {
  logger.debug() << "deactivation";

  if (reason != Controller::ReasonNone) {
    // Just show that we're disconnected
    // we're doing the actual disconnect once
    // the vpn-service has the new server ready in Action->Activate
    emit disconnected();
    logger.warning() << "deactivation skipped for Switching";
    return;
  }
  AndroidVPNActivity::sendToService(ServiceAction::ACTION_DEACTIVATE,
                                    QString());
}

void AndroidController::checkStatus() {
  logger.debug() << "check status";

  AndroidVPNActivity::sendToService(ServiceAction::ACTION_REQUEST_STATISTIC,
                                    QString());
}

void AndroidController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  QString cacheFolderPath =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  auto cacheFolder = QDir(cacheFolderPath);
  QFile logFile(cacheFolder.absoluteFilePath("mozilla_deamon_logs.txt"));
  if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    a_callback(QString());
    return;
  }
  auto content = logFile.readAll();
  a_callback(QString::fromUtf8(content));
}

void AndroidController::cleanupBackendLogs() {
  logger.debug() << "cleanup logs";
  AndroidVPNActivity::sendToService(ServiceAction::ACTION_REQUEST_CLEANUP_LOG,
                                    QString());
}
