/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "telemetry.h"

#include "leakdetector.h"
#include "logger.h"
#include "modules/modulevpn.h"
#include "mozillavpn.h"
#include "telemetry/gleansample.h"

constexpr int CONNECTION_STABILITY_MSEC = 45000;

namespace {
Logger logger("Telemetry");
}

Telemetry::Telemetry() {
  MVPN_COUNT_CTOR(Telemetry);

  m_connectionStabilityTimer.setSingleShot(true);
  connect(&m_connectionStabilityTimer, &QTimer::timeout, this,
          &Telemetry::connectionStabilityEvent);

#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
  connect(&m_gleanControllerUpTimer, &QTimer::timeout, this,
          &Telemetry::periodicStateRecorder);
  m_gleanControllerUpTimer.start(
      Constants::controllerPeriodicStateRecorderMsec());
  m_gleanControllerUpTimer.setSingleShot(false);
#endif
}

Telemetry::~Telemetry() { MVPN_COUNT_DTOR(Telemetry); }

void Telemetry::initialize() {
  logger.debug() << "Initialize";

  Controller* controller = ModuleVPN::instance()->controller();
  Q_ASSERT(controller);

  connect(controller, &Controller::handshakeFailed, this,
          [](const QString& publicKey) {
            logger.info() << "Send a handshake failure event";

            emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
                GleanSample::connectivityHandshakeTimeout,
                {{"server", publicKey},
                 {"transport", ModuleVPN::instance()
                                   ->networkWatcher()
                                   ->getCurrentTransport()}});
          });

  connect(controller, &Controller::stateChanged, this, [this]() {
    Controller* controller = ModuleVPN::instance()->controller();
    Q_ASSERT(controller);
    Controller::State state = controller->state();

    if (state != Controller::StateOn) {
      m_connectionStabilityTimer.stop();
    } else {
      m_connectionStabilityTimer.start(CONNECTION_STABILITY_MSEC);
    }

    emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
        GleanSample::controllerStep,
        {{"state", QVariant::fromValue(state).toString()}});
    // Specific events for on and off state to aid with analysis
    if (state == Controller::StateOn) {
      emit MozillaVPN::instance()->recordGleanEvent(
          GleanSample::controllerStateOn);
    }
    if (state == Controller::StateOff) {
      emit MozillaVPN::instance()->recordGleanEvent(
          GleanSample::controllerStateOff);
    }
  });

  connect(controller, &Controller::readyToServerUnavailable, this, []() {
    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::serverUnavailableError);
  });
}

void Telemetry::connectionStabilityEvent() {
  logger.info() << "Send a connection stability event";

  Controller* controller = ModuleVPN::instance()->controller();
  Q_ASSERT(controller);
  Q_ASSERT(controller->state() == Controller::StateOn);

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::connectivityStable,
      {{"server",
        MozillaVPN::instance()->currentServer()->exitServerPublicKey()},
       {"latency",
        QString::number(ModuleVPN::instance()->connectionHealth()->latency())},
       {"loss",
        QString::number(ModuleVPN::instance()->connectionHealth()->loss())},
       {"stddev",
        QString::number(ModuleVPN::instance()->connectionHealth()->stddev())},
       {"transport",
        ModuleVPN::instance()->networkWatcher()->getCurrentTransport()}});
}

#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
void Telemetry::periodicStateRecorder() {
  // On mobile this is handled seperately in a background process
  Controller* controller = ModuleVPN::instance()->controller();
  Q_ASSERT(controller);

  Controller::State controllerState = controller->state();

  if (controllerState == Controller::StateOn) {
    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::controllerStateOn);
  }
  if (controllerState == Controller::StateOff) {
    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::controllerStateOff);
  }
}
#endif
