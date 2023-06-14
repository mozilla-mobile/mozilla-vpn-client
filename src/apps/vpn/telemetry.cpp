/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "telemetry.h"

#include "appconstants.h"
#include "apppermission.h"
#include "connectionhealth.h"
#include "controller.h"
#include "dnshelper.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "gleandeprecated.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"
#include "purchasehandler.h"
#include "settingsholder.h"
#include "telemetry/gleansample.h"

#if defined(MZ_ANDROID)
#  include "platforms/android/androidvpnactivity.h"
#endif

#include <QJsonDocument>
#include <QJsonValue>

constexpr int CONNECTION_STABILITY_MSEC = 45000;

constexpr const uint32_t VPNSESSION_PING_TIMER_SEC = 3 * 60 * 60;  // 3 hours
constexpr const uint32_t VPNSESSION_PING_TIMER_DEBUG_SEC = 120;

namespace {
Logger logger("Telemetry");
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
      AppConstants::controllerPeriodicStateRecorderMsec());
  m_gleanControllerUpTimer.setSingleShot(false);
#endif
}

Telemetry::~Telemetry() { MZ_COUNT_DTOR(Telemetry); }

void Telemetry::initialize() {
  logger.debug() << "Initialize";

  MozillaVPN* vpn = MozillaVPN::instance();
  connect(vpn, &MozillaVPN::stateChanged, this, []() {
    int state = MozillaVPN::instance()->state();

    if (state > App::StateCustom) {
      mozilla::glean::sample::app_step.record(
          mozilla::glean::sample::AppStepExtra{
              ._state = QVariant::fromValue(
                            static_cast<MozillaVPN::CustomState>(state))
                            .toString()});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::appStep,
          {{"state",
            QVariant::fromValue(static_cast<MozillaVPN::CustomState>(state))
                .toString()}});
    } else {
      mozilla::glean::sample::app_step.record(
          mozilla::glean::sample::AppStepExtra{
              ._state = QVariant::fromValue(static_cast<App::State>(state))
                            .toString()});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::appStep,
          {{"state",
            QVariant::fromValue(static_cast<App::State>(state)).toString()}});
    }

    if (state == MozillaVPN::StateDeviceLimit) {
      mozilla::glean::sample::max_device_reached.record();
      emit GleanDeprecated::instance()->recordGleanEvent(
          GleanSample::maxDeviceReached);
    }

    if (state == App::StateSubscriptionNotValidated) {
      mozilla::glean::sample::iap_subscription_failed.record(
          mozilla::glean::sample::IapSubscriptionFailedExtra{
              ._error = "not-validated",
              ._sku = PurchaseHandler::instance()->currentSKU()});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::iapSubscriptionFailed,
          {{"error", "not-validated"},
           {"sku", PurchaseHandler::instance()->currentSKU()}});
    }

    if (state == App::StateSubscriptionBlocked) {
      mozilla::glean::sample::iap_subscription_failed.record(
          mozilla::glean::sample::IapSubscriptionFailedExtra{
              ._error = "alrady-subscribed",
          });
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::iapSubscriptionFailed, {{"error", "alrady-subscribed"}});
    }
  });

  connect(vpn, &MozillaVPN::authenticationStarted, this, []() {
    mozilla::glean::sample::authentication_started.record();
    emit GleanDeprecated::instance()->recordGleanEvent(
        GleanSample::authenticationStarted);
  });

  connect(vpn, &MozillaVPN::authenticationAborted, this, []() {
    mozilla::glean::sample::authentication_aborted.record();
    emit GleanDeprecated::instance()->recordGleanEvent(
        GleanSample::authenticationAborted);
  });

  connect(vpn, &MozillaVPN::authenticationCompleted, this, []() {
    mozilla::glean::sample::authentication_completed.record();
    emit GleanDeprecated::instance()->recordGleanEvent(
        GleanSample::authenticationCompleted);
  });

  connect(vpn, &MozillaVPN::deviceRemoved, this, [](const QString& source) {
    mozilla::glean::sample::device_removed.record(
        mozilla::glean::sample::DeviceRemovedExtra{._source = source});
    emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
        GleanSample::deviceRemoved, {{"source", source}});
  });

  Controller* controller = vpn->controller();
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

  connect(
      SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this,
      []() {
        bool currentSetting = SettingsHolder::instance()->startAtBoot();
        mozilla::glean::settings::connect_on_startup_active.set(currentSetting);
      });

  PurchaseHandler* purchaseHandler = PurchaseHandler::instance();
  connect(purchaseHandler, &PurchaseHandler::subscriptionStarted, this,
          [](const QString& productIdentifier) {
            mozilla::glean::sample::iap_subscription_started.record(
                mozilla::glean::sample::IapSubscriptionStartedExtra{
                    ._sku = productIdentifier});
            emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
                GleanSample::iapSubscriptionStarted,
                {{"sku", productIdentifier}});
          });

  connect(purchaseHandler, &PurchaseHandler::restoreSubscriptionStarted, this,
          []() {
            mozilla::glean::sample::iap_restore_sub_started.record();
            emit GleanDeprecated::instance()->recordGleanEvent(
                GleanSample::iapRestoreSubStarted);
          });

  connect(MozillaVPN::instance(), &MozillaVPN::logSubscriptionCompleted, this,
          []() {
            mozilla::glean::sample::iap_subscription_completed.record(
                mozilla::glean::sample::IapSubscriptionCompletedExtra{
                    ._sku = PurchaseHandler::instance()->currentSKU()});
            emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
                GleanSample::iapSubscriptionCompleted,
                {{"sku", PurchaseHandler::instance()->currentSKU()}});
          });

  connect(purchaseHandler, &PurchaseHandler::subscriptionFailed, this, []() {
    mozilla::glean::sample::iap_subscription_failed.record(
        mozilla::glean::sample::IapSubscriptionFailedExtra{
            ._error = "failed",
            ._sku = PurchaseHandler::instance()->currentSKU()});
    emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
        GleanSample::iapSubscriptionFailed,
        {{"error", "failed"},
         {"sku", PurchaseHandler::instance()->currentSKU()}});
  });

  connect(purchaseHandler, &PurchaseHandler::subscriptionCanceled, this, []() {
    mozilla::glean::sample::iap_subscription_failed.record(
        mozilla::glean::sample::IapSubscriptionFailedExtra{
            ._error = "canceled",
            ._sku = PurchaseHandler::instance()->currentSKU()});
    emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
        GleanSample::iapSubscriptionFailed,
        {{"error", "canceled"},
         {"sku", PurchaseHandler::instance()->currentSKU()}});
  });

  connect(controller, &Controller::stateChanged, this, [this, controller]() {
    if (Feature::get(Feature::Feature_superDooperMetrics)->isSupported()) {
      if (controller->state() == Controller::StateOn) {
        mozilla::glean_pings::Vpnsession.submit("flush");

        QString sessionId =
            mozilla::glean::session::session_id.generateAndSet();
        mozilla::glean::session::session_start.set();
        mozilla::glean::session::dns_type.set(DNSHelper::getDNSType());
        mozilla::glean::session::apps_excluded.set(
            AppPermission::instance()->disabledAppCount());

        mozilla::glean_pings::Vpnsession.submit("start");
        m_vpnSessionPingTimer.start(
            (SettingsHolder::instance()->vpnSessionPingTimeoutDebug()
                 ? VPNSESSION_PING_TIMER_DEBUG_SEC
                 : VPNSESSION_PING_TIMER_SEC) *
            1000);
      }
    }
  });

  connect(
      controller, &Controller::controllerDisconnected, this,
      [this, controller]() {
        if (Feature::get(Feature::Feature_superDooperMetrics)->isSupported()) {
          if (controller->state() == Controller::StateOff) {
            mozilla::glean::session::session_end.set();

            // This generateAndSet must be called after submission of ping.
            // When doing VPN-4443 ensure it comes after the submission.

            // We rotating the UUID here as a safety measure. It is rotated
            // again before the next session start, and we expect to see the
            // UUID created here in only one ping: The session ping with a
            // "flush" reason, which should contain this UUID and no other
            // metrics.
            QString sessionId =
                mozilla::glean::session::session_id.generateAndSet();

            mozilla::glean_pings::Vpnsession.submit("end");
            m_vpnSessionPingTimer.stop();
          }
        }
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
