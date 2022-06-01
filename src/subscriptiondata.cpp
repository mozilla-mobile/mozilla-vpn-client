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
}

SubscriptionData::SubscriptionData() { MVPN_COUNT_CTOR(SubscriptionData); }

SubscriptionData::~SubscriptionData() { MVPN_COUNT_DTOR(SubscriptionData); }

bool SubscriptionData::fromJson(const QByteArray& json) {
  logger.debug() << "Subscription from JSON start";

  if (!json.isEmpty() && m_rawJson == json) {
    logger.debug() << "Data has not changed";
    return true;
  }

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  // Subscription
  m_createdAt = obj.value("created_at").toInt();
  if (!m_createdAt) {
    return false;
  }

  m_expiresOn = obj.value("expires_on").toInt();
  if (!m_expiresOn) {
    return false;
  }

  QJsonValue isCancelled = obj.value("is_cancelled");
  if (!isCancelled.isBool()) {
    return false;
  }
  m_isCancelled = isCancelled.toBool();

  QJsonValue status = obj.value("status");
  if (!status.isString()) {
    return false;
  }
  m_status = status.toString();

  QJsonValue type = obj.value("type");
  if (!type.isString()) {
    return false;
  }
  m_type = type.toString();

  // Plan
  QJsonObject planData = obj.value("plan").toObject();

  m_planAmount = planData.value("amount").toInt();
  if (!m_planAmount && m_planAmount != 0) {
    return false;
  }
  
  QJsonValue planCurrency = planData.value("currency");
  if (!planCurrency.isString()) {
    return false;
  }
  m_planCurrency = planCurrency.toString();

  m_planIntervalCount = planData.value("interval_count").toInt();
  if (!m_planIntervalCount) {
    return false;
  }

  // Payment
  QJsonObject paymentData = obj.value("payment").toObject();

  QJsonValue paymentType = paymentData.value("type");
  if (!paymentType.isString()) {
    return false;
  }

  m_paymentType = paymentType.toString();

  if (m_paymentType == "credit") {
    QJsonValue creditCardBrand = paymentData.value("credit_card_brand");
    if (!creditCardBrand.isString()) {
      return false;
    }
    m_creditCardBrand = creditCardBrand.toString();

    QJsonValue creditCardLast4 = paymentData.value("credit_card_last4");
    if (!creditCardBrand.isString()) {
      return false;
    }
    m_creditCardLast4 = creditCardLast4.toString();

    m_creditCardExpMonth = paymentData.value("credit_card_exp_month").toInt();
    if (!m_creditCardExpMonth) {
      return false;
    }

    m_creditCardExpYear = paymentData.value("credit_card_exp_year").toInt();
    if (!m_creditCardExpYear) {
      return false;
    }
  }

  logger.debug() << "Subscription from JSON ready";

  m_rawJson = json;
  emit changed();

  return true;
}
