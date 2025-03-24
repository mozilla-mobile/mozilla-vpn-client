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
#include "controller.h"
#include "errorhandler.h"
#include "feature/feature.h"
#include "glean/generated/metrics.h"
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
        Q_UNUSED(parcelBody);
        emit connected(m_serverPublicKey);
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
  connect(
      activity, &AndroidVPNActivity::eventOnboardingCompleted, this,
      [this]() { emit disconnected(); }, Qt::QueuedConnection);
  connect(
      activity, &AndroidVPNActivity::eventVpnConfigPermissionResponse, this,
      [](bool granted) {
        Controller* controller = MozillaVPN::instance()->controller();
        controller->startHandshakeTimer();

        granted
            ? mozilla::glean::outcome::onboarding_ntwrk_perm_granted.record()
            : mozilla::glean::outcome::onboarding_ntwrk_perm_denied.record();
      },
      Qt::QueuedConnection);
}
AndroidController::~AndroidController() { MZ_COUNT_DTOR(AndroidController); }

void AndroidController::initialize(const Device* device, const Keys* keys) {
  logger.debug() << "Initializing";

  Q_UNUSED(keys);

  m_deviceName = device->name();
  m_devicePublicKey = device->publicKey();
  m_deviceCreationTime = device->createdAt().toMSecsSinceEpoch();

  AndroidVPNActivity::connectService();
}

void AndroidController::activate(const InterfaceConfig& config,
                                 Controller::Reason reason) {
  Q_ASSERT(config.m_hopType == InterfaceConfig::SingleHop);
  logger.debug() << "Activation";

  m_serverPublicKey = config.m_serverPublicKey;

  // Serialise arguments for the VPNService
  QJsonObject jDevice;
  jDevice["publicKey"] = m_devicePublicKey;
  jDevice["name"] = m_deviceName;
  jDevice["createdAt"] = m_deviceCreationTime;
  jDevice["ipv4Address"] = config.m_deviceIpv4Address;
  jDevice["ipv6Address"] = config.m_deviceIpv6Address;

  QJsonObject jKeys;
  jKeys["privateKey"] = config.m_privateKey;

  logger.info() << "Server" << logger.sensitive(config.m_serverPublicKey);
  // This could be done better with VpnService.Builder.excludeRoute()
  // but that requires API level 33 (Android 13).
  const auto lanRoutes = Controller::getExcludedIPAddressRanges();
  QJsonArray jAllowedIPs;
  foreach (auto item, config.m_allowedIPAddressRanges) {
    if (item.prefixLength() > 0) {
      jAllowedIPs.append(QJsonValue(item.toString()));
    } else if (item.type() == QAbstractSocket::IPv4Protocol) {
      QList<IPAddress> list = {item};
      foreach (auto prefix, IPAddress::excludeAddresses(list, lanRoutes.v4)) {
        jAllowedIPs.append(QJsonValue(prefix.toString()));
      }
    } else if (item.type() == QAbstractSocket::IPv6Protocol) {
      QList<IPAddress> list = {item};
      foreach (auto prefix, IPAddress::excludeAddresses(list, lanRoutes.v6)) {
        jAllowedIPs.append(QJsonValue(prefix.toString()));
      }
    }
  }

  QJsonArray excludedApps;
  foreach (auto appID, config.m_vpnDisabledApps) {
    excludedApps.append(QJsonValue(appID));
  }

  QJsonObject jMainServer;
  jMainServer["ipv4AddrIn"] = config.m_serverIpv4AddrIn;
  jMainServer["ipv4Gateway"] = config.m_serverIpv4Gateway;
  jMainServer["ipv6AddrIn"] = config.m_serverIpv6AddrIn;
  jMainServer["ipv6Gateway"] = config.m_serverIpv6Gateway;
  jMainServer["publicKey"] = config.m_serverPublicKey;
  jMainServer["port"] = (double)config.m_serverPort;

  // Find a fallback servers in the same location in case
  // the original one becomes unstable / unavailable
  auto vpn = MozillaVPN::instance();
  QList<Server> fallbackServers =
      vpn->controller()->currentServer().backupServers(
          config.m_serverPublicKey);
  QJsonArray jServers;

  jServers.append(jMainServer);
  foreach (auto fallbackServer, fallbackServers) {
    QJsonObject jFallbackServer;
    jFallbackServer["ipv4AddrIn"] = fallbackServer.ipv4AddrIn();
    jFallbackServer["ipv4Gateway"] = fallbackServer.ipv4Gateway();
    jFallbackServer["ipv6AddrIn"] = fallbackServer.ipv6AddrIn();
    jFallbackServer["ipv6Gateway"] = fallbackServer.ipv6Gateway();
    jFallbackServer["publicKey"] = fallbackServer.publicKey();
    jFallbackServer["port"] = (double)fallbackServer.choosePort();
    jServers.append(jFallbackServer);
  }

  QJsonObject args;
  args["device"] = jDevice;
  args["keys"] = jKeys;
  args["servers"] = jServers;
  args["reason"] = (int)reason;
  args["allowedIPs"] = jAllowedIPs;
  args["excludedApps"] = excludedApps;
  args["dns"] = config.m_dnsServer;

  // Build the "canned" Notification messages
  // They will be used in case this config will be re-enabled
  // to show the appropriate notification messages
  QString localizedCityName =
      vpn->controller()->currentServer().localizedExitCityName();
  args["city"] = localizedCityName;

  QJsonObject messages;
  messages["productName"] =
      I18nStrings::instance()->t(I18nStrings::ProductName);
  messages["connectedHeader"] = I18nStrings::instance()->t(
      I18nStrings::NotificationsVPNConnectedTitle);  // Connected
  messages["connectedBody"] =
      I18nStrings::instance()
          ->t(I18nStrings::NotificationsVPNConnectedMessages)
          .arg(localizedCityName);
  messages["disconnectedHeader"] = I18nStrings::instance()->t(
      I18nStrings::NotificationsVPNDisconnectedTitle);
  messages["disconnectedBody"] =
      I18nStrings::instance()
          ->t(I18nStrings::NotificationsVPNDisconnectedMessage)
          .arg(localizedCityName);
  args["messages"] = messages;

  args["isSuperDooperFeatureActive"] =
      Feature::get(Feature::Feature_superDooperMetrics)->isSupported();
  args["installationId"] = config.m_installationId;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  args["gleanDebugTag"] = settingsHolder->gleanDebugTagActive()
                              ? settingsHolder->gleanDebugTag()
                              : "";
  args["isUsingShortTimerSessionPing"] =
      settingsHolder->shortTimerSessionPing();

  args["isOnboarding"] = reason == Controller::ReasonOnboarding;

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

void AndroidController::forceDaemonSilentServerSwitch() {
  logger.debug() << "requesting silent server switch from controller";
  AndroidVPNActivity::sendToService(ServiceAction::ACTION_SILENT_SERVER_SWITCH,
                                    QString());
}
