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
      [this]() {
        auto vpn = MozillaVPN::instance();
        if (vpn->state() == App::StateOnboarding) {
          vpn->onboardingCompleted();
          emit disconnected();
        }
      },
      Qt::QueuedConnection);
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

  QJsonObject jServer;
  logger.info() << "Server" << logger.sensitive(config.m_serverPublicKey);
  jServer["ipv4AddrIn"] = config.m_serverIpv4AddrIn;
  jServer["ipv4Gateway"] = config.m_serverIpv4Gateway;
  jServer["ipv6AddrIn"] = config.m_serverIpv6AddrIn;
  jServer["ipv6Gateway"] = config.m_serverIpv6Gateway;

  jServer["publicKey"] = config.m_serverPublicKey;
  jServer["port"] = (double)config.m_serverPort;

  // TODO: This could be done better with VpnService.Builder.excludeRoute()
  // we just need a way to get this config down that far.
  const auto localRoutes = Controller::getExcludedIPAddressRanges().flatten();
  const auto fullAllowedIPs =
      IPAddress::excludeAddresses(config.m_allowedIPAddressRanges, localRoutes);
  QJsonArray jAllowedIPs;
  foreach (auto item, fullAllowedIPs) {
    jAllowedIPs.append(QJsonValue(item.toString()));
  }

  QJsonArray excludedApps;
  foreach (auto appID, config.m_vpnDisabledApps) {
    excludedApps.append(QJsonValue(appID));
  }

  // Find a Server as Fallback in the Same Location in case
  // the original one becomes unstable / unavailable
  auto vpn = MozillaVPN::instance();
  Server fallbackServer =
      vpn->controller()->currentServer().backupServer(config.m_serverPublicKey);
  QJsonObject jFallbackServer;
  if (fallbackServer.initialized()) {
    jFallbackServer["ipv4AddrIn"] = fallbackServer.ipv4AddrIn();
    jFallbackServer["ipv4Gateway"] = fallbackServer.ipv4Gateway();
    jFallbackServer["ipv6AddrIn"] = fallbackServer.ipv6AddrIn();
    jFallbackServer["ipv6Gateway"] = fallbackServer.ipv6Gateway();

    jFallbackServer["publicKey"] = fallbackServer.publicKey();
    jFallbackServer["port"] = (double)fallbackServer.choosePort();
  }

  QJsonObject args;
  args["device"] = jDevice;
  args["keys"] = jKeys;
  args["server"] = jServer;
  args["reason"] = (int)reason;
  args["allowedIPs"] = jAllowedIPs;
  args["excludedApps"] = excludedApps;
  args["dns"] = config.m_dnsServer;
  if (fallbackServer.initialized()) {
    args["serverFallback"] = jFallbackServer;
  }
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

  args["isOnboarding"] =
      MozillaVPN::instance()->state() == App::StateOnboarding;

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
