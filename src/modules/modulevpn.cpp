/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulevpn.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "modules/modulevpn/taskcontrolleraction.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "taskscheduler.h"

#include <QGuiApplication>
#include <QJsonObject>
#include <QQmlEngine>

namespace {
ModuleVPN* s_instance = nullptr;
Logger logger(LOG_MAIN, "ModuleVPN");
}  // namespace

ModuleVPN::ModuleVPN(QObject* parent) : Module(parent) {
  MVPN_COUNT_CTOR(ModuleVPN);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

ModuleVPN::~ModuleVPN() {
  MVPN_COUNT_DTOR(ModuleVPN);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
ModuleVPN* ModuleVPN::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

void ModuleVPN::initialize() {
  connect(
      qApp, &QGuiApplication::commitDataRequest, this,
      [this]() {
#if QT_VERSION < 0x060000
        qApp->setFallbackSessionManagementEnabled(false);
#endif
        deactivate();
      },
      Qt::DirectConnection);

  connect(&m_controller, &Controller::stateChanged, this,
          &ModuleVPN::controllerStateChanged);

  connect(&m_controller, &Controller::stateChanged, NetworkManager::instance(),
          &NetworkManager::clearCache);

  connect(&m_controller, &Controller::readyToUpdate, this,
          &ModuleVPN::readyToUpdate);

  connect(&m_controller, &Controller::readyToQuit, this,
          &ModuleVPN::readyToQuit, Qt::QueuedConnection);

  connect(&m_controller, &Controller::readyToBackendFailure, this,
          &ModuleVPN::readyToBackendFailure);

  connect(&m_controller, &Controller::readyToServerUnavailable, this,
          [](bool pingReceived) {
            NotificationHandler::instance()->serverUnavailableNotification(
                pingReceived);
          });

  connect(MozillaVPN::instance(), &MozillaVPN::stateChanged, this, []() {
    if (MozillaVPN::instance()->state() != MozillaVPN::StateMain) {
      // We don't call deactivate() because that is meant to be used for
      // UI interactions only and it deletes all the pending tasks.
      TaskScheduler::scheduleTask(
          new TaskControllerAction(TaskControllerAction::eDeactivate));
    } else {
      ModuleVPN::instance()->controller()->initialize();
    }
  });

  m_captivePortalDetection.initialize();

  m_connectionBenchmark.initialize();

  m_connectionHealth.initialize();

  m_networkWatcher.initialize();

  m_serverLatency.initialize();

  m_telemetry.initialize();

  registerInspectorCommands();
}

QJSValue ModuleVPN::captivePortalDetectionValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_captivePortalDetection;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(
      engine->newQMetaObject(&CaptivePortalDetection::staticMetaObject));
  return value;
}

QJSValue ModuleVPN::connectionBenchmarkValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_connectionBenchmark;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(
      engine->newQMetaObject(&ConnectionBenchmark::staticMetaObject));
  return value;
}

QJSValue ModuleVPN::connectionHealthValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_connectionHealth;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(
      engine->newQMetaObject(&ConnectionHealth::staticMetaObject));
  return value;
}

QJSValue ModuleVPN::controllerValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_controller;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&Controller::staticMetaObject));
  return value;
}

void ModuleVPN::activate() {
  logger.debug() << "VPN tunnel activation";

  TaskScheduler::deleteTasks();

  // We are about to connect. If the device key needs to be regenerated, this
  // is the right time to do it.
  MozillaVPN::instance()->maybeRegenerateDeviceKey();

  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eActivate));
}

void ModuleVPN::deactivate() {
  logger.debug() << "VPN tunnel deactivation";

  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eDeactivate));
}

void ModuleVPN::silentSwitch() {
  logger.debug() << "VPN tunnel silent server switch";

  // Let's delete all the tasks before running the silent-switch op. If we are
  // here, the connection does not work and we don't want to wait for timeouts
  // to run the silenct-switch.
  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eSilentSwitch));
}

void ModuleVPN::controllerStateChanged() {
  logger.debug() << "Controller state changed";

  if (!m_controllerInitialized) {
    m_controllerInitialized = true;

    if (SettingsHolder::instance()->startAtBoot()) {
      logger.debug() << "Start on boot";
      activate();
    }
  }

  maybeShowNotification();
}

bool ModuleVPN::validateUserDNS(const QString& dns) const {
  return DNSHelper::validateUserDNS(dns);
}

void ModuleVPN::registerInspectorCommands() {
  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "force_captive_portal_check", "Force a captive portal check", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        ModuleVPN::instance()->captivePortalDetection()->detectCaptivePortal();
        return QJsonObject();
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "force_captive_portal_detection", "Simulate a captive portal detection",
      0, [](InspectorHandler*, const QList<QByteArray>&) {
        ModuleVPN::instance()
            ->captivePortalDetection()
            ->captivePortalDetected();
        ModuleVPN::instance()->controller()->captivePortalPresent();
        return QJsonObject();
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "force_unsecured_network", "Force an unsecured network detection", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        ModuleVPN::instance()->networkWatcher()->unsecuredNetwork("Dummy",
                                                                  "Dummy");
        return QJsonObject();
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "activate", "Activate the VPN", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        ModuleVPN::instance()->activate();
        return QJsonObject();
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "deactivate", "Deactivate the VPN", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        ModuleVPN::instance()->deactivate();
        return QJsonObject();
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "connection_health_unsettled",
      "Retrieve the unsettled status of the connection health", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        QJsonObject obj;
        obj["value"] = ModuleVPN::instance()->connectionHealth()->isUnsettled();
        return obj;
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "connection_benchmark_property",
      "Retrieve a property value from the connection benchmark object", 1,
      [](InspectorHandler*, const QList<QByteArray>& arguments) {
        QJsonObject obj;
        obj["value"] = ModuleVPN::instance()
                           ->connectionBenchmark()
                           ->property(arguments[1])
                           .toString();
        return obj;
      }});

  InspectorHandler::registerCommand(InspectorHandler::InspectorCommand{
      "connection_benchmark_url",
      "Set the URL for the next connection benchmark test", 1,
      [](InspectorHandler*, const QList<QByteArray>& arguments) {
        ModuleVPN::instance()->connectionBenchmark()->setDownloadUrl(
            arguments[1]);
        return QJsonObject();
      }});
}

void ModuleVPN::updateRequired() { m_controller.updateRequired(); }

void ModuleVPN::quit() { m_controller.quit(); }

void ModuleVPN::backendFailure() { m_controller.backendFailure(); }

void ModuleVPN::serializeLogs(QTextStream* out,
                              std::function<void()>&& a_finalizeCallback) {
  std::function<void()> finalizeCallback = std::move(a_finalizeCallback);

  m_controller.getBackendLogs(
      [out,
       finalizeCallback = std::move(finalizeCallback)](const QString& logs) {
        logger.debug() << "Logs from the backend service received";

        *out << Qt::endl
             << Qt::endl
             << "Mozilla VPN backend logs" << Qt::endl
             << "========================" << Qt::endl
             << Qt::endl;

        if (!logs.isEmpty()) {
          *out << logs;
        } else {
          *out << "No logs from the backend.";
        }
        *out << Qt::endl;
        *out << "==== SETTINGS ====" << Qt::endl;
        *out << SettingsHolder::instance()->getReport();
        *out << "==== DEVICE ====" << Qt::endl;
        *out << Device::currentDeviceReport();
        *out << Qt::endl;

        finalizeCallback();
      });
}

void ModuleVPN::cleanupLogs() { m_controller.cleanupBackendLogs(); }

void ModuleVPN::maybeShowNotification() {
  logger.debug() << "Maybe show notification";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() != MozillaVPN::StateMain &&
      // The Disconnected notification should be triggerable
      // on StateInitialize, in case the user was connected during a log-out
      // Otherwise existing notifications showing "connected" would update
      !(vpn->state() == MozillaVPN::StateInitialize &&
        ModuleVPN::instance()->controller()->state() == Controller::StateOff)) {
    return;
  }

  QString title;
  QString message;
  QString countryCode = vpn->currentServer()->exitCountryCode();
  QString localizedCityName = vpn->currentServer()->localizedCityName();
  QString localizedCountryName =
      vpn->serverCountryModel()->localizedCountryName(countryCode);

  switch (ModuleVPN::instance()->controller()->state()) {
    case Controller::StateOn:
      m_connected = true;

      if (m_switching) {
        m_switching = false;

        if (!SettingsHolder::instance()->serverSwitchNotification()) {
          // Dont show notification if it's turned off.
          return;
        }

        QString localizedPreviousExitCountryName =
            vpn->serverCountryModel()->localizedCountryName(
                vpn->currentServer()->previousExitCountryCode());
        QString localizedPreviousExitCityName =
            vpn->currentServer()->localizedPreviousExitCityName();

        if ((localizedPreviousExitCountryName == localizedCountryName) &&
            (localizedPreviousExitCityName == localizedCityName)) {
          // Don't show notifications unless the exit server changed, see:
          // https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1719
          return;
        }

        //% "VPN Switched Servers"
        title = qtTrId("vpn.systray.statusSwitch.title");
        //% "Switched from %1, %2 to %3, %4"
        //: Shown as message body in a notification. %1 and %3 are countries, %2
        //: and %4 are cities.
        message = qtTrId("vpn.systray.statusSwtich.message")
                      .arg(localizedPreviousExitCountryName,
                           localizedPreviousExitCityName, localizedCountryName,
                           localizedCityName);
      } else {
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }
        //% "VPN Connected"
        title = qtTrId("vpn.systray.statusConnected.title");
        //% "Connected to %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is
        //: the city.
        message = qtTrId("vpn.systray.statusConnected.message")
                      .arg(localizedCountryName, localizedCityName);
      }
      break;

    case Controller::StateOff:
      if (m_connected) {
        m_connected = false;
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }

        //% "VPN Disconnected"
        title = qtTrId("vpn.systray.statusDisconnected.title");
        //% "Disconnected from %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is
        //: the city.
        message = qtTrId("vpn.systray.statusDisconnected.message")
                      .arg(localizedCountryName, localizedCityName);
      }
      break;

    case Controller::StateSwitching:
      m_connected = true;
      m_switching = true;
      break;

    default:
      break;
  }

  Q_ASSERT(title.isEmpty() == message.isEmpty());

  if (!title.isEmpty()) {
    emit notificationNeeded(NotificationHandler::None, title, message, 2000);
  }
}
