/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptiondata.h"
#include "leakdetector.h"
#include "logger.h"

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
  if (!m_planAmount && m_planAmount != 0) {
    return false;
  }

  QJsonValue planCurrency = planData["currency"];
  if (!planCurrency.isString()) {
    return false;
  }
  // We transform currency code to uppercase in order to be compliant with
  // ISO 4216. FxA passes the lowercase currency code values without
  // transformation from Stripe.
  m_planCurrency = planCurrency.toString().toUpper();

  m_planIntervalCount = planData["interval_count"].toInt();
  if (!m_planIntervalCount) {
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
    QJsonValue paymentProvider = paymentData["payment_provider"];
    // We should always get a payment provider if there is payment data
    if (!paymentProvider.isString()) {
      return false;
    }
    m_paymentProvider = paymentProvider.toString();

    // Payment type
    QJsonValue paymentType = paymentData["payment_type"];
    if (paymentType.isString()) {
      m_paymentType = paymentType.toString();

      // For credit cards we also show card details
      if (m_paymentType == "credit") {
        QJsonValue creditCardBrand = paymentData["brand"];
        if (!creditCardBrand.isString()) {
          return false;
        }
        m_creditCardBrand = creditCardBrand.toString();

        QJsonValue creditCardLast4 = paymentData["last4"];
        if (!creditCardBrand.isString()) {
          return false;
        }
        m_creditCardLast4 = creditCardLast4.toString();

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
  }

  // Subscription
  QJsonObject subscriptionData = obj["subscription"].toObject();

  QJsonValue type = subscriptionData["_subscription_type"];
  if (!type.isString()) {
    return false;
  }

  // Enum from string
  m_type = static_cast<TypeSubscription>(
      QMetaEnum::fromType<TypeSubscription>().keyToValue(
          type.toString().toUtf8()));

  // Parse subscription data depending on subscription platform
  switch (m_type) {
    case web:
      if (!parseSubscriptionDataWeb(subscriptionData)) {
        return false;
      }
      break;
    case iap_apple:
      [[fallthrough]];
    case iap_google:
      if (!parseSubscriptionDataIap(subscriptionData)) {
        return false;
      }
      break;
    default:
      logger.error() << "No matching subscription type" << type.toString();
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

  m_expiresOn = subscriptionData["expiry_time_millis"].toInt();
  if (!m_expiresOn) {
    return false;
  }
  QJsonValue autoRenewing = subscriptionData["auto_renewing"];
  if (!autoRenewing.isBool()) {
    return false;
  }
  m_isCancelled = !autoRenewing.toBool();

  return true;
}

bool SubscriptionData::parseSubscriptionDataWeb(
    const QJsonObject& subscriptionData) {
  logger.debug() << "Parse web start";

  m_createdAt = subscriptionData["created"].toInt();
  if (!m_createdAt) {
    return false;
  }
  m_expiresOn = subscriptionData["current_period_end"].toInt();
  if (!m_expiresOn) {
    return false;
  }
  QJsonValue isCancelled = subscriptionData["cancel_at_period_end"];
  if (!isCancelled.isBool()) {
    return false;
  }
  m_isCancelled = isCancelled.toBool();
  QJsonValue status = subscriptionData["status"];
  m_status = subscriptionData["status"].toString();
  if (m_status.isEmpty()) {
    return false;
  }
  m_status = status.toString();

  return true;
}
