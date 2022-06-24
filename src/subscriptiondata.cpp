/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptiondata.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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
  QJsonObject planData = obj.value("plan").toObject();

  m_planAmount = planData.value("amount").toInt();
  if (!m_planAmount && m_planAmount != 0) {
    return false;
  }

  QJsonValue planCurrency = planData.value("currency");
  if (!planCurrency.isString()) {
    return false;
  }
  // We transform currency code to uppercase in order to be compliant with
  // ISO 4216. FxA passes the lowercase currency code values without
  // transformation from Stripe.
  m_planCurrency = planCurrency.toString().toUpper();

  m_planIntervalCount = planData.value("interval_count").toInt();
  if (!m_planIntervalCount) {
    return false;
  }
  logger.debug() << "Parse plan ready";

  // Payment
  logger.debug() << "Parse payment start";
  QJsonObject paymentData = obj.value("payment").toObject();

  // If we do not receive payment data from FxA we don’t show it instead of
  // throwing an error. There is a known bug with FxA which causes FxA to not
  // show payment info: https://mozilla-hub.atlassian.net/browse/FXA-3856.
  if (!paymentData.isEmpty()) {
    // Payment provider
    QJsonValue paymentProvider = paymentData.value("payment_provider");
    // We should always get a payment provider if there is payment data
    if (!paymentProvider.isString()) {
      return false;
    }
    m_paymentProvider = paymentProvider.toString();

    // Payment type
    QJsonValue paymentType = paymentData.value("payment_type");
    if (paymentType.isString()) {
      m_paymentType = paymentType.toString();

      // For credit cards we also show card details
      if (m_paymentType == "credit") {
        QJsonValue creditCardBrand = paymentData.value("brand");
        if (!creditCardBrand.isString()) {
          return false;
        }
        m_creditCardBrand = creditCardBrand.toString();

        QJsonValue creditCardLast4 = paymentData.value("last4");
        if (!creditCardBrand.isString()) {
          return false;
        }
        m_creditCardLast4 = creditCardLast4.toString();

        m_creditCardExpMonth =
            paymentData.value("exp_month").toInt();
        if (!m_creditCardExpMonth) {
          return false;
        }

        m_creditCardExpYear = paymentData.value("exp_year").toInt();
        if (!m_creditCardExpYear) {
          return false;
        }
      }
    }
  }
  logger.debug() << "Parse payment ready";

  // Subscription
  logger.debug() << "Parse subscription start";
  QJsonObject subscriptionData = obj.value("subscription").toObject();

  QJsonValue type = subscriptionData.value("_subscription_type");
  if (!type.isString()) {
    return false;
  }
  m_type = type.toString();

  // Parse subscription data depending on subscription platform
  if (m_type == "web") {
    if(!parseSubscriptionDataWeb(subscriptionData)) { return false; }
  } else if (m_type == "iap_apple" || m_type == "iap_google") {
    if(!parseSubscriptionDataIap(subscriptionData)) { return false; }
  } else {
    logger.error() << "No matching subscription type" << m_type;
    return false;
  }
  logger.debug() << "Parse subscription ready";

  m_rawJson = json;
  emit changed();
  logger.debug() << "Subscription data from JSON ready";

  return true;
}

bool SubscriptionData::parseSubscriptionDataIap(const QJsonObject& obj) {
  Q_UNUSED(obj);
  logger.debug() << "Parse IAP start" << m_type;

  m_expiresOn = obj.value("expiry_time_millis").toInt();
  if (!m_expiresOn) {
    return false;
  }
  QJsonValue autoRenewing = obj.value("auto_renewing");
  if (!autoRenewing.isBool()) {
    return false;
  }
  m_isCancelled = !autoRenewing.toBool();

  logger.debug() << "Parse IAP ready";
  return false;
}

bool SubscriptionData::parseSubscriptionDataWeb(const QJsonObject& obj) {
  logger.debug() << "Parse web start";

  m_createdAt = obj.value("created").toInt();
  if (!m_createdAt) {
    return false;
  }
  m_expiresOn = obj.value("current_period_end").toInt();
  if (!m_expiresOn) {
    return false;
  }
  QJsonValue isCancelled = obj.value("cancel_at_period_end");
  if (!isCancelled.isBool()) {
    return false;
  }
  m_isCancelled = isCancelled.toBool();
  QJsonValue status = obj.value("status");
  if (!status.isString()) {
    return false;
  }
  m_status = status.toString();

  logger.debug() << "Parse web ready";
  return true;
}
