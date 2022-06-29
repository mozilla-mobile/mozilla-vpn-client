/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptiondata.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "telemetry/gleansample.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>

namespace {
Logger logger(LOG_MODEL, "SubscriptionData");
}  // namespace

SubscriptionData::SubscriptionData() { MVPN_COUNT_CTOR(SubscriptionData); }

SubscriptionData::~SubscriptionData() { MVPN_COUNT_DTOR(SubscriptionData); }

bool SubscriptionData::fromJson(const QByteArray& json) {
  logger.debug() << "Subscription data from JSON start";

  if (!json.isEmpty() && m_rawJson == json) {
    logger.debug() << "Data has not changed";
    return true;
  }

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

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
             !Constants::inProduction()) {
    // For testing purposes we support additional intervals
    // and use a monthly plan as fallback
    planIntervalMonths = 1;
  } else {
    logger.error() << "Unexpected interval type:" << planInterval;
    emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
        GleanSample::unhandledSubPlanInterval, {{"interval", planInterval}});
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
      emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
          GleanSample::unhandledSubPlanInterval,
          {{"interval", planInterval}, {"interval_count", planIntervalCount}});
      return false;
  }

  // We transform the currency code to uppercase in order to be compliant with
  // ISO 4216. FxA passes the lowercase currency code values without
  // transformation from Stripe.
  m_planCurrency = planData["currency"].toString().toUpper();
  if (m_planCurrency.isEmpty()) {
    return false;
  }

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
    m_type = SubscriptionApple;
    if (!parseSubscriptionDataIap(subscriptionData)) {
      return false;
    }
  } else if (type == "iap_google") {
    m_type = SubscriptionGoogle;
    if (!parseSubscriptionDataIap(subscriptionData)) {
      return false;
    }
  } else {
    logger.error() << "No matching subscription type" << type;
    return false;
  }

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

    // Payment type
    m_paymentType = paymentData["payment_type"].toString();

    // For credit cards we also show card details
    if (m_paymentType == "credit") {
      m_creditCardBrand = paymentData["brand"].toString();
      if (m_creditCardBrand.isEmpty()) {
        return false;
      }

      m_creditCardLast4 = paymentData["last4"].toString();
      if (m_creditCardLast4.isEmpty()) {
        return false;
      }

      m_creditCardExpMonth = paymentData["exp_month"].toInt();
      if (!m_creditCardExpMonth) {
        return false;
      }

      m_creditCardExpYear = paymentData["exp_year"].toInt();
      if (!m_creditCardExpYear) {
        return false;
      }
    }
  }

  m_rawJson = json;
  emit changed();
  logger.debug() << "Subscription data from JSON ready";

  return true;
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

  if (subscriptionStatus == "active") {
    m_status = Active;
  } else if (subscriptionStatus == "inactive") {
    m_status = Inactive;
  } else {
    logger.error() << "Unexpected subscription status:" << subscriptionStatus;
    return false;
  }

  return true;
}
