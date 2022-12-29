/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "telemetry.h"

#include "appconstants.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "telemetry/gleansample.h"

constexpr int CONNECTION_STABILITY_MSEC = 45000;

namespace {
Logger logger("Telemetry");
}

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

  connect(controller, &Controller::handshakeFailed, this,
          [](const QString& publicKey) {
            logger.info() << "Send a handshake failure event";

            mozilla::glean::sample::connectivity_handshake_timeout.record(
                mozilla::glean::sample::ConnectivityHandshakeTimeoutExtra{
                    ._server = publicKey,
                    ._transport = MozillaVPN::instance()
                                      ->networkWatcher()
                                      ->getCurrentTransport()});
            emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
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
    emit vpn->recordGleanEventWithExtraKeys(
        GleanSample::controllerStep,
        {{"state", QVariant::fromValue(state).toString()}});
    // Specific events for on and off state to aid with analysis
    if (state == Controller::StateOn) {
      mozilla::glean::sample::controller_state_on.record();
      emit vpn->recordGleanEvent(GleanSample::controllerStateOn);
    }
    if (state == Controller::StateOff) {
      mozilla::glean::sample::controller_state_off.record();
      emit vpn->recordGleanEvent(GleanSample::controllerStateOff);
    }
  });

  connect(controller, &Controller::readyToServerUnavailable, this, []() {
    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    mozilla::glean::sample::server_unavailable_error.record();
    emit vpn->recordGleanEvent(GleanSample::serverUnavailableError);
  });
}

void Telemetry::connectionStabilityEvent() {
  logger.info() << "Send a connection stability event";

  MozillaVPN* vpn = MozillaVPN::instance();

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);
  Q_ASSERT(controller->state() == Controller::StateOn);

  mozilla::glean::sample::connectivity_stable.record(
      mozilla::glean::sample::ConnectivityStableExtra{
          ._latency = QString::number(vpn->connectionHealth()->latency()),
          ._loss = QString::number(vpn->connectionHealth()->loss()),
          ._server = vpn->currentServer()->exitServerPublicKey(),
          ._stddev = QString::number(vpn->connectionHealth()->stddev()),
          ._transport = vpn->networkWatcher()->getCurrentTransport()});
  emit vpn->recordGleanEventWithExtraKeys(
      GleanSample::connectivityStable,
      {{"server", vpn->currentServer()->exitServerPublicKey()},
       {"latency", QString::number(vpn->connectionHealth()->latency())},
       {"loss", QString::number(vpn->connectionHealth()->loss())},
       {"stddev", QString::number(vpn->connectionHealth()->stddev())},
       {"transport", vpn->networkWatcher()->getCurrentTransport()}});
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
    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::controllerStateOn);
  }
  if (controllerState == Controller::StateOff) {
    mozilla::glean::sample::controller_state_off.record();
    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::controllerStateOff);
  }
}
#endif
