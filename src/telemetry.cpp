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
}

Telemetry::~Telemetry() { MZ_COUNT_DTOR(Telemetry); }

void Telemetry::initialize() {
  logger.debug() << "Initialize";

  MozillaVPN* vpn = MozillaVPN::instance();
  connect(vpn, &MozillaVPN::logSubscriptionCompleted, this,
          [] { mozilla::glean::outcome::subscription_completed.record(); });

  connect(vpn, &MozillaVPN::stateChanged, this, []() {
    int state = MozillaVPN::instance()->state();

    if (state == App::StateOnboarding) {
      if (!SettingsHolder::instance()->onboardingStarted()) {
        mozilla::glean::outcome::onboarding_started.record();
      }
    }
  });

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);

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
}

void Telemetry::vpnSessionPingTimeout() {
  if (Feature::get(Feature::Feature_superDooperMetrics)->isSupported()) {
    mozilla::glean_pings::Vpnsession.submit("timer");
  }
}

// The Glean timer id for the performance.time_to_main_screen metric.
static qint64 s_timeToFirstScreenTimerId = 0;

void Telemetry::startTimeToFirstScreenTimer() {
  logger.info() << "Start performance.time_to_main_screen timer";

  s_timeToFirstScreenTimerId =
      mozilla::glean::performance::time_to_main_screen.start();
}

void Telemetry::stopTimeToFirstScreenTimer() {
  logger.info() << "Stop performance.time_to_main_screen timer";

  mozilla::glean::performance::time_to_main_screen.stopAndAccumulate(
      s_timeToFirstScreenTimerId);
}
