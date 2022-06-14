/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "telemetry.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#include "telemetry/gleansample.h"

constexpr int CONNECTION_STABILITY_MSEC = 45000;

namespace {
Logger logger(LOG_MAIN, "Telemetry");
}

Telemetry::Telemetry() {
  MVPN_COUNT_CTOR(Telemetry);

  m_connectionStabilityTimer.setSingleShot(true);
  connect(&m_connectionStabilityTimer, &QTimer::timeout, this,
          &Telemetry::connectionStabilityEvent);

  connect(&m_gleanControllerUpTimer, &QTimer::timeout, this,
          &Telemetry::periodicStateRecorder);
  m_gleanControllerUpTimer.start(
      Constants::controllerPeriodicStateRecorderMsec());
  m_gleanControllerUpTimer.setSingleShot(false);
}

Telemetry::~Telemetry() { MVPN_COUNT_DTOR(Telemetry); }

void Telemetry::initialize() {
  logger.debug() << "Initialize";

  Controller* controller = MozillaVPN::instance()->controller();
  Q_ASSERT(controller);

  connect(controller, &Controller::handshakeFailed, this,
          [](const QString& publicKey) {
            logger.info() << "Send a handshake failure event";

            emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
                GleanSample::connectivityHandshakeTimeout,
                {{"server", publicKey}});
          });

  connect(controller, &Controller::stateChanged, this, [this]() {
    if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
      m_connectionStabilityTimer.stop();
    } else {
      m_connectionStabilityTimer.start(CONNECTION_STABILITY_MSEC);
    }
  });
}

void Telemetry::connectionStabilityEvent() {
  logger.info() << "Send a connection stability event";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);
  Q_ASSERT(controller->state() == Controller::StateOn);

  emit vpn->recordGleanEventWithExtraKeys(
      GleanSample::connectivityStable,
      {
          {"server", vpn->serverPublicKey()},
          {"latency", QString::number(vpn->connectionHealth()->latency())},
          {"loss", QString::number(vpn->connectionHealth()->loss())},
          {"stddev", QString::number(vpn->connectionHealth()->stddev())},
      });
}

void Telemetry::periodicStateRecorder() {
// On mobile this is handled seperately in a background process
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);
  Controller* controller = vpn->controller();
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
#endif
}
