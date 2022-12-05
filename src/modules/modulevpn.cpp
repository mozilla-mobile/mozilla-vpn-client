/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulevpn.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "modules/modulevpn/taskcontrolleraction.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "taskscheduler.h"

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
  connect(&m_controller, &Controller::stateChanged, this,
          &ModuleVPN::controllerStateChanged);

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
