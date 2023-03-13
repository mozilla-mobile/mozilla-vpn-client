/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "telemetry.h"

#include "appconstants.h"
#include "connectionhealth.h"
#include "controller.h"
#include "glean/generated/metrics.h"
#include "gleandeprecated.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"
#include "settingsholder.h"
#include "telemetry/gleansample.h"

#if defined(MZ_ANDROID)
#  include "platforms/android/androidvpnactivity.h"
#endif

#include <QJsonDocument>
#include <QJsonValue>

constexpr int CONNECTION_STABILITY_MSEC = 45000;

namespace {
Logger logger("Telemetry");
}  // namespace

Telemetry::Telemetry() {
  MZ_COUNT_CTOR(Telemetry);

  m_connectionStabilityTimer.setSingleShot(true);
  connect(&m_connectionStabilityTimer, &QTimer::timeout, this,
          &Telemetry::connectionStabilityEvent);

#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS)
  connect(&m_gleanControllerUpTimer, &QTimer::timeout, this,
          &Telemetry::periodicStateRecorder);
  m_gleanControllerUpTimer.start(
      AppConstants::controllerPeriodicStateRecorderMsec());
  m_gleanControllerUpTimer.setSingleShot(false);
#endif
}

Telemetry::~Telemetry() { MZ_COUNT_DTOR(Telemetry); }

void Telemetry::initialize() {
  logger.debug() << "Initialize";

  Controller* controller = MozillaVPN::instance()->controller();
  Q_ASSERT(controller);

#if defined(MZ_ANDROID)
  connect(AndroidVPNActivity::instance(), &AndroidVPNActivity::eventInitialized,
          this, &Telemetry::onDaemonStatus);
#endif

  connect(controller, &Controller::handshakeFailed, this,
          [](const QString& publicKey) {
            logger.info() << "Send a handshake failure event";

            mozilla::glean::sample::connectivity_handshake_timeout.record(
                mozilla::glean::sample::ConnectivityHandshakeTimeoutExtra{
                    ._server = publicKey,
                    ._transport = MozillaVPN::instance()
                                      ->networkWatcher()
                                      ->getCurrentTransport()});
            emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
                GleanSample::connectivityHandshakeTimeout,
                {{"server", publicKey},
                 {"transport", MozillaVPN::instance()
                                   ->networkWatcher()
                                   ->getCurrentTransport()}});
          });

  connect(controller, &Controller::stateChanged, this, [this]() {
    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);
    Controller* controller = vpn->controller();
    Q_ASSERT(controller);
    Controller::State state = controller->state();

    if (state != Controller::StateOn) {
      m_connectionStabilityTimer.stop();
    } else {
      m_connectionStabilityTimer.start(CONNECTION_STABILITY_MSEC);
    }

    mozilla::glean::sample::controller_step.record(
        mozilla::glean::sample::ControllerStepExtra{
            ._state = QVariant::fromValue(state).toString()});
    emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
        GleanSample::controllerStep,
        {{"state", QVariant::fromValue(state).toString()}});
    // Specific events for on and off state to aid with analysis
    if (state == Controller::StateOn) {
      mozilla::glean::sample::controller_state_on.record();
      emit GleanDeprecated::instance()->recordGleanEvent(
          GleanSample::controllerStateOn);
    }
    if (state == Controller::StateOff) {
      mozilla::glean::sample::controller_state_off.record();
      emit GleanDeprecated::instance()->recordGleanEvent(
          GleanSample::controllerStateOff);
    }
  });

  connect(controller, &Controller::readyToServerUnavailable, this, []() {
    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    mozilla::glean::sample::server_unavailable_error.record();
    emit GleanDeprecated::instance()->recordGleanEvent(
        GleanSample::serverUnavailableError);
  });

  connect(SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this, []() {
    bool currentSetting = SettingsHolder::instance()->startAtBoot();
    mozilla::glean::performance::connect_on_startup_active.set(currentSetting);
  });
}

void Telemetry::connectionStabilityEvent() {
  logger.info() << "Send a connection stability event";

  MozillaVPN* vpn = MozillaVPN::instance();

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);
  Q_ASSERT(controller->state() == Controller::StateOn);

  // We use Controller->currentServer because the telemetry event should record
  // the location in use by the Controller and not MozillaVPN::serverData, which
  // could have changed in the meantime.
  mozilla::glean::sample::connectivity_stable.record(
      mozilla::glean::sample::ConnectivityStableExtra{
          ._latency = QString::number(vpn->connectionHealth()->latency()),
          ._loss = QString::number(vpn->connectionHealth()->loss()),
          ._server = vpn->controller()->currentServer().exitServerPublicKey(),
          ._stddev = QString::number(vpn->connectionHealth()->stddev()),
          ._transport = vpn->networkWatcher()->getCurrentTransport()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::connectivityStable,
      {{"server", vpn->controller()->currentServer().exitServerPublicKey()},
       {"latency", QString::number(vpn->connectionHealth()->latency())},
       {"loss", QString::number(vpn->connectionHealth()->loss())},
       {"stddev", QString::number(vpn->connectionHealth()->stddev())},
       {"transport", vpn->networkWatcher()->getCurrentTransport()}});
}

void Telemetry::startTimeToFirstScreenTimer() {
  logger.info() << "Start performance.time_to_main_screen timer";

  m_timeToFirstScreenTimerId =
      mozilla::glean::performance::time_to_main_screen.start();
}

void Telemetry::stopTimeToFirstScreenTimer() {
  logger.info() << "Stop performance.time_to_main_screen timer";

  mozilla::glean::performance::time_to_main_screen.stopAndAccumulate(
      m_timeToFirstScreenTimerId);
}

#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS)
void Telemetry::periodicStateRecorder() {
  // On mobile this is handled seperately in a background process
  MozillaVPN* vpn = MozillaVPN::instance();
  Controller* controller = vpn->controller();
  Q_ASSERT(controller);

  Controller::State controllerState = controller->state();

  if (controllerState == Controller::StateOn) {
    mozilla::glean::sample::controller_state_on.record();
    emit GleanDeprecated::instance()->recordGleanEvent(
        GleanSample::controllerStateOn);
  }
  if (controllerState == Controller::StateOff) {
    mozilla::glean::sample::controller_state_off.record();
    emit GleanDeprecated::instance()->recordGleanEvent(
        GleanSample::controllerStateOff);
  }
}
#endif

#if defined(MZ_ANDROID)
void Telemetry::onDaemonStatus(const QString& data) {
  auto doc = QJsonDocument::fromJson(data.toUtf8());
  bool connected = doc.object()["connected"].toBool(false);
  if (!connected) {
    // If we're not connected, connection health is irrelevant.
    return;
  }
  auto status = doc.object()["connection-health-status"].toString();
  if (status.isNull()) {
    return;
  }

  mozilla::glean::sample::android_daemon_status.record(
      mozilla::glean::sample::AndroidDaemonStatusExtra{._connectionHealthState =
                                                           status});
}
#endif
