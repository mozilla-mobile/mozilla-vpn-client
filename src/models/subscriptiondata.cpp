/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptiondata.h"
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

  // Get interval enum from string
  bool okInterval = false;
  TypePlanInterval planIntervalType = static_cast<TypePlanInterval>(
      QMetaEnum::fromType<TypePlanInterval>().keyToValue(
          planInterval.toUpper().toUtf8(), &okInterval));
  if (!okInterval) {
    logger.error() << "Unsupported interval type:" << planInterval;
    return false;
  }

  // Convert `interval` to number of months
  int planIntervalMonths;
  switch (planIntervalType) {
    case MONTH:
      planIntervalMonths = 1;
      break;
    case YEAR:
      planIntervalMonths = 12;
      break;
    default:
      logger.error() << "Unhandled interval type:" << planIntervalType;
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
      logger.error() << "Unhandled billing interval:"
                     << planIntervalMonthsTotal;
      emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
          GleanSample::unhandledSubPlanInterval,
          {{"interval", planIntervalType},
           {"interval_count", planIntervalCount}});
      return false;
  }

  // We transform the currency code to uppercase in order to be compliant with
  // ISO 4216. FxA passes the lowercase currency code values without
  // transformation from Stripe.
  m_planCurrency = planData["currency"].toString().toUpper();
  if (m_planCurrency.isEmpty()) {
    return false;
  }
  logger.debug() << "Parse plan ready";

  // Payment
  QJsonObject paymentData = obj["payment"].toObject();

  // If we do not receive payment data from FxA we don’t show it instead of
  // throwing an error. There is a known bug with FxA which causes FxA to not
  // show payment info: https://mozilla-hub.atlassian.net/browse/FXA-3856.
  if (!paymentData.isEmpty()) {
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

  // Subscription
  QJsonObject subscriptionData = obj["subscription"].toObject();

  QString type = subscriptionData["_subscription_type"].toString();
  if (type.isEmpty()) {
    return false;
  }

  // Enum from string
  bool okType = false;
  m_type = static_cast<TypeSubscription>(
      QMetaEnum::fromType<TypeSubscription>().keyToValue(
          type.toUpper().toUtf8(), &okType));
  if (!okType) {
    logger.error() << "Unsupported subscription type:" << type;
    return false;
  }

  // Parse subscription data depending on subscription platform
  switch (m_type) {
    case WEB:
      if (!parseSubscriptionDataWeb(subscriptionData)) {
        return false;
      }
      break;
    case IAP_APPLE:
      [[fallthrough]];
    case IAP_GOOGLE:
      if (!parseSubscriptionDataIap(subscriptionData)) {
        return false;
      }
      break;
    default:
      logger.error() << "No matching subscription type" << type;
      return false;
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
    m_status = ACTIVE;
  } else {
    m_status = INACTIVE;
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

  // Get status enum from string
  bool okStatus = false;
  TypeStatus subscriptionStatusType =
      static_cast<TypeStatus>(QMetaEnum::fromType<TypeStatus>().keyToValue(
          subscriptionStatus.toUpper().toUtf8(), &okStatus));
  if (!okStatus) {
    logger.error() << "Unsupported subscription status:"
                   << subscriptionStatusType;
    return false;
  }
  m_status = subscriptionStatusType;

  return true;
}
