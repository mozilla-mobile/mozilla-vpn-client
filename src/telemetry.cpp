/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "telemetry.h"

#include "apppermission.h"
#include "connectionhealth.h"
#include "constants.h"
#include "controller.h"
#include "dnshelper.h"
#include "feature/feature.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"
#include "purchasehandler.h"
#include "settingsholder.h"

#if defined(MZ_ANDROID)
#  include "platforms/android/androidvpnactivity.h"
#endif

#include <QJsonDocument>
#include <QJsonValue>

constexpr const uint32_t VPNSESSION_PING_TIMER_SEC = 3 * 60 * 60;  // 3 hours
constexpr const uint32_t SHORT_DEBUG_VPNSESSION_PING_TIMER_SEC =
    3 * 60;  // 3 minutes

namespace {
Logger logger("Telemetry");
using namespace std::chrono_literals;
constexpr auto CONNECTION_STABILITY = 45s;

}  // namespace

Telemetry::Telemetry() {
  MZ_COUNT_CTOR(Telemetry);

  m_connectionStabilityTimer.setSingleShot(true);
  connect(&m_connectionStabilityTimer, &QTimer::timeout, this,
          &Telemetry::connectionStabilityEvent);

  connect(&m_vpnSessionPingTimer, &QTimer::timeout, this,
          &Telemetry::vpnSessionPingTimeout);

#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS)
  connect(&m_gleanControllerUpTimer, &QTimer::timeout, this,
          &Telemetry::periodicStateRecorder);
  m_gleanControllerUpTimer.start(
      Constants::Timers::controllerRecordPeriodicState());
  m_gleanControllerUpTimer.setSingleShot(false);
#endif
}

Telemetry::~Telemetry() { MZ_COUNT_DTOR(Telemetry); }

void Telemetry::initialize() {
  logger.debug() << "Initialize";

  MozillaVPN* vpn = MozillaVPN::instance();
  connect(vpn, &MozillaVPN::logSubscriptionCompleted, this,
          [] { mozilla::glean::outcome::subscription_completed.record(); });

  connect(vpn, &MozillaVPN::stateChanged, this, []() {
    int state = MozillaVPN::instance()->state();

    if (state > App::StateCustom) {
      mozilla::glean::sample::app_step.record(
          mozilla::glean::sample::AppStepExtra{
              ._state = QVariant::fromValue(
                            static_cast<MozillaVPN::CustomState>(state))
                            .toString()});
    } else {
      mozilla::glean::sample::app_step.record(
          mozilla::glean::sample::AppStepExtra{
              ._state = QVariant::fromValue(static_cast<App::State>(state))
                            .toString()});
    }

    if (state == MozillaVPN::StateDeviceLimit) {
      mozilla::glean::sample::max_device_reached.record();
    }

    if (state == App::StateOnboarding) {
      if (!SettingsHolder::instance()->onboardingStarted()) {
        mozilla::glean::outcome::onboarding_started.record();
      }
    }
  });

  connect(vpn, &MozillaVPN::authenticationStarted, this,
          []() { mozilla::glean::sample::authentication_started.record(); });

  connect(vpn, &MozillaVPN::authenticationAborted, this,
          []() { mozilla::glean::sample::authentication_aborted.record(); });

  connect(vpn, &MozillaVPN::authenticationCompleted, this,
          []() { mozilla::glean::sample::authentication_completed.record(); });

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);

  connect(
      controller, &Controller::handshakeFailed, this,
      [](const QString& publicKey) {
        logger.info() << "Send a handshake failure event";
        mozilla::glean::sample::connectivity_handshake_timeout.record(
            mozilla::glean::sample::ConnectivityHandshakeTimeoutExtra{
                ._server = publicKey,
                ._transport = QVariant::fromValue(MozillaVPN::instance()
                                                      ->networkWatcher()
                                                      ->getReachability())});
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
      m_connectionStabilityTimer.start(CONNECTION_STABILITY);
    }

    mozilla::glean::sample::controller_step.record(
        mozilla::glean::sample::ControllerStepExtra{
            ._state = QVariant::fromValue(state).toString()});
    // Specific events for on and off state to aid with analysis
    if (state == Controller::StateOn) {
      mozilla::glean::sample::controller_state_on.record();
    }
    if (state == Controller::StateOff) {
      mozilla::glean::sample::controller_state_off.record();
    }
  });

  connect(controller, &Controller::readyToServerUnavailable, this, []() {
    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    mozilla::glean::sample::server_unavailable_error.record();
  });

  connect(
      SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this,
      []() {
        bool currentSetting = SettingsHolder::instance()->startAtBoot();
        mozilla::glean::settings::connect_on_startup_active.set(currentSetting);
      });

  connect(
      controller, &Controller::recordConnectionStartTelemetry, this,
      [this, controller]() {
        if (Feature::get(Feature::Feature_superDooperMetrics)->isSupported()) {
          if (controller->state() == Controller::StateOn) {
            mozilla::glean_pings::Vpnsession.submit("flush");

            mozilla::glean::session::session_id.generateAndSet();
            mozilla::glean::session::session_start.set();
            mozilla::glean::session::dns_type.set(DNSHelper::getDNSType());
            mozilla::glean::session::apps_excluded.set(
                AppPermission::instance()->disabledAppCount());

            mozilla::glean_pings::Vpnsession.submit("start");

            if (SettingsHolder::instance()->shortTimerSessionPing()) {
              m_vpnSessionPingTimer.start(
                  SHORT_DEBUG_VPNSESSION_PING_TIMER_SEC * 1000);
            } else {
              m_vpnSessionPingTimer.start(VPNSESSION_PING_TIMER_SEC * 1000);
            }
          }
        }
      });

  connect(
      controller, &Controller::recordConnectionEndTelemetry, this, [this]() {
        if (Feature::get(Feature::Feature_superDooperMetrics)->isSupported()) {
          mozilla::glean::session::session_end.set();
          mozilla::glean_pings::Vpnsession.submit("end");
          m_vpnSessionPingTimer.stop();

          // We are rotating the UUID here as a safety measure. It is rotated
          // again before the next session start, and we expect to see the
          // UUID created here in only one ping: The session ping with a
          // "flush" reason, which should contain this UUID and no other
          // metrics.
          mozilla::glean::session::session_id.generateAndSet();
        }
      });

#ifndef MZ_MOBILE
  connect(controller, &Controller::recordDataTransferTelemetry, this,
          [controller]() {
// On mobile, these metrics are recorded in the daemon process.
            Q_ASSERT(controller);

            controller->getStatus([](const QString& serverIpv4Gateway,
                                     const QString& deviceIpv4Address,
                                     uint64_t txBytes, uint64_t rxBytes) {
              Q_UNUSED(serverIpv4Gateway);
              Q_UNUSED(deviceIpv4Address);

              mozilla::glean::connection_health::data_transferred_tx
                  .accumulate_single_sample(txBytes);
              mozilla::glean::connection_health::data_transferred_rx
                  .accumulate_single_sample(rxBytes);
            });
          });
#endif
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
          ._transport =
              QVariant::fromValue(vpn->networkWatcher()->getReachability())});
}

void Telemetry::vpnSessionPingTimeout() {
  if (Feature::get(Feature::Feature_superDooperMetrics)->isSupported()) {
    mozilla::glean_pings::Vpnsession.submit("timer");
  }
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
  }
  if (controllerState == Controller::StateOff) {
    mozilla::glean::sample::controller_state_off.record();
  }
}
#endif
