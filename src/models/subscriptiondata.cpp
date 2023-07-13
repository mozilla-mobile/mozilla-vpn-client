/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptiondata.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>

#include "constants.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/location.h"
#include "mozillavpn.h"
#include "settingsholder.h"

namespace {
Logger logger("SubscriptionData");
}  // namespace

SubscriptionData::SubscriptionData() { MZ_COUNT_CTOR(SubscriptionData); }

SubscriptionData::~SubscriptionData() { MZ_COUNT_DTOR(SubscriptionData); }

bool SubscriptionData::fromJson(const QByteArray& json) {
  logger.debug() << "Subscription data from JSON start";

  if (!json.isEmpty() && m_rawJson == json) {
    logger.debug() << "Data has not changed";
    return true;
  }

  if (!fromJsonInternal(json)) {
    return false;
  }

  m_rawJson = json;
  emit changed();
  return true;
}

bool SubscriptionData::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading the subscription data from settings";

  const QByteArray& json = settingsHolder->devices();
  if (json.isEmpty() || !fromJsonInternal(json)) {
    return false;
  }

  m_rawJson = json;
  return true;
}

bool SubscriptionData::fromJsonInternal(const QByteArray& json) {
  resetData();

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  // Subscription
  QJsonObject subscriptionData = obj["subscription"].toObject();

  QString type = subscriptionData["_subscription_type"].toString();
  if (type.isEmpty()) {
    return false;
  }

  // Parse subscription data depending on subscription platform
  if (type == "web") {
    m_type = SubscriptionWeb;
    if (!parseSubscriptionDataWeb(subscriptionData)) {
      return false;
    }
  } else if (type == "iap_apple") {
    // TODO: Parse subscription data as soon as FxA includes Apple subscriptions
    // in their API response.
    m_type = SubscriptionApple;
    if (!parseSubscriptionDataIap(subscriptionData)) {
      return false;
    }

    // For Apple subscriptions that is all the information we currently have.
    logger.debug() << "Subscription data from JSON ready";
    return true;
  } else if (type == "iap_google") {
    m_type = SubscriptionGoogle;
    if (!parseSubscriptionDataIap(subscriptionData)) {
      return false;
    }
  } else {
    logger.error() << "No matching subscription type" << type;
    return false;
  }

  // Plan
  logger.debug() << "Parse plan start";
  QJsonObject planData = obj["plan"].toObject();

  m_planAmount = planData["amount"].toInt();
  if (!m_planAmount) {
    return false;
  }

  // Billing frequency
  QString planInterval = planData["interval"].toString();
  if (planInterval.isEmpty()) {
    return false;
  }

  // Convert `interval` to number of months
  int planIntervalMonths;
  if (planInterval == "year") {
    planIntervalMonths = 12;
  } else if (planInterval == "month") {
    planIntervalMonths = 1;
  } else if ((planInterval == "week" || planInterval == "day") &&
             !AppConstants::inProduction()) {
    // For testing purposes we support additional intervals
    // and use a monthly plan as fallback
    planIntervalMonths = 1;
  } else {
    logger.error() << "Unexpected interval type:" << planInterval;

    mozilla::glean::sample::unhandled_sub_plan_interval.record(
        mozilla::glean::sample::UnhandledSubPlanIntervalExtra{
            ._interval = planInterval});
    return false;
  }

  // Number of intervals between subscription billings
  int planIntervalCount = planData["interval_count"].toInt();
  if (!planIntervalCount) {
    return false;
  }

  // Get total billing interval in months
  int planIntervalMonthsTotal = planIntervalMonths * planIntervalCount;
  // Set the plan interval
  switch (planIntervalMonthsTotal) {
    case 1:
      m_planBillingInterval = BillingIntervalMonthly;
      break;
    case 6:
      m_planBillingInterval = BillingIntervalHalfYearly;
      break;
    case 12:
      m_planBillingInterval = BillingIntervalYearly;
      break;
    default:
      logger.error() << "Unexpected billing interval:"
                     << planIntervalMonthsTotal;

      mozilla::glean::sample::unhandled_sub_plan_interval.record(
          mozilla::glean::sample::UnhandledSubPlanIntervalExtra{
              ._interval = planInterval, ._intervalCount = planIntervalCount});
      return false;
  }

  // We transform the currency code to uppercase in order to be compliant with
  // ISO 4216. FxA passes the lowercase currency code values without
  // transformation from Stripe.
  m_planCurrency = planData["currency"].toString().toUpper();
  if (m_planCurrency.isEmpty()) {
    return false;
  }

  // Payments made using USD or CAD are subject to tax
  m_planRequiresTax = QStringList({"USD", "CAD"}).contains(m_planCurrency);

  // Payment
  QJsonObject paymentData = obj["payment"].toObject();

  // If we do not receive payment data from FxA we don’t show it instead of
  // throwing an error. There is a known bug with FxA which causes FxA to not
  // show payment info: https://mozilla-hub.atlassian.net/browse/FXA-3856.
  if (!paymentData.isEmpty() && m_type == SubscriptionWeb) {
    // Payment provider
    m_paymentProvider = paymentData["payment_provider"].toString();
    // We should always get a payment provider if there is payment data
    if (m_paymentProvider.isEmpty()) {
      return false;
    }

    QJsonArray subscriptionsList = paymentData["subscriptions"].toArray();
    for (const QJsonValue& subscriptionValue : subscriptionsList) {
      QJsonObject subscription = subscriptionValue.toObject();
      if (subscription["product_id"] ==
          AppConstants::privacyBundleProductId()) {
        m_isPrivacyBundleSubscriber = true;
        break;
      }
    }

    // We show card details only for stripe
    if (m_paymentProvider == "stripe") {
      m_creditCardBrand = paymentData["brand"].toString();
      m_creditCardLast4 = paymentData["last4"].toString();
      m_creditCardExpMonth = paymentData["exp_month"].toInt();
      m_creditCardExpYear = paymentData["exp_year"].toInt();
    }
  }

  logger.debug() << "Subscription data from JSON ready";
  return true;
}

void SubscriptionData::writeSettings() {
  SettingsHolder::instance()->setSubscriptionData(m_rawJson);
}

bool SubscriptionData::parseSubscriptionDataIap(
    const QJsonObject& subscriptionData) {
  logger.debug() << "Parse IAP start" << m_type;

  m_expiresOn = subscriptionData["expiry_time_millis"].toVariant().toLongLong();
  if (!m_expiresOn) {
    return false;
  }
  m_isCancelled = !subscriptionData["auto_renewing"].toBool();

  quint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now <= m_expiresOn) {
    m_status = Active;
  } else {
    m_status = Inactive;
  }

  return true;
}

bool SubscriptionData::parseSubscriptionDataWeb(
    const QJsonObject& subscriptionData) {
  logger.debug() << "Parse web start";

  // We receive the values for `created` and `current_period_end in seconds.
  m_createdAt = subscriptionData["created"].toVariant().toLongLong() * 1000;

  if (!m_createdAt) {
    return false;
  }
  m_expiresOn =
      subscriptionData["current_period_end"].toVariant().toLongLong() * 1000;
  if (!m_expiresOn) {
    return false;
  }

  m_isCancelled = subscriptionData["cancel_at_period_end"].toBool();
  QString subscriptionStatus = subscriptionData["status"].toString();
  if (subscriptionStatus.isEmpty()) {
    return false;
  }

  if (subscriptionStatus == "active" || subscriptionStatus == "trialing") {
    m_status = Active;
  } else {
    m_status = Inactive;
  }

  return true;
}

void SubscriptionData::resetData() {
  logger.debug() << "Reset data";
  m_rawJson.clear();

  m_type = SubscriptionUnknown;
  m_status = Inactive;
  m_createdAt = 0;
  m_expiresOn = 0;
  m_isCancelled = false;
  m_isPrivacyBundleSubscriber = false;
  m_planRequiresTax = false;

  m_planBillingInterval = BillingIntervalUnknown;
  m_planAmount = 0;
  m_planCurrency.clear();

  m_paymentProvider.clear();
  m_creditCardBrand.clear();
  m_creditCardLast4.clear();
  m_creditCardExpMonth = 0;
  m_creditCardExpYear = 0;
}
