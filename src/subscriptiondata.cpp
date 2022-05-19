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
  logger.debug() << "Subscription from JSON";

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
  QJsonValue createdAt = obj.value("created_at");
  QJsonValue expiresOn = obj.value("expires_on");
  QJsonValue isCancelled = obj.value("is_cancelled");
  if (!isCancelled.isBool()) {
    return false;
  }

  QJsonValue status = obj.value("status");
  if (!status.isString()) {
    return false;
  }

  QJsonValue type = obj.value("type");
  if (!type.isString()) {
    return false;
  }

  m_createdAt = createdAt.toInt();
  m_expiresOn = expiresOn.toInt();
  m_isCancelled = isCancelled.toBool();
  m_status = status.toString();
  m_type = type.toString();

  // Plan
  QJsonObject planData = obj.value("plan").toObject();

  QJsonValue planAmount = planData.value("amount");
  
  QJsonValue planCurrency = planData.value("currency");
  if (!planCurrency.isString()) {
    return false;
  }

  QJsonValue planIntervalCount = planData.value("interval_count");

  m_planAmount = planAmount.toInt();
  m_planCurrency = planCurrency.toString();
  m_planIntervalCount = planIntervalCount.toInt();

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

    QJsonValue creditCardLast4 = paymentData.value("credit_card_last4");
    if (!creditCardBrand.isString()) {
      return false;
    }

    QJsonValue creditCardExpMonth = paymentData.value("credit_card_exp_month");
    QJsonValue creditCardExpYear = paymentData.value("credit_card_exp_year");

    m_creditCardBrand = creditCardBrand.toString();
    m_creditCardLast4 = creditCardLast4.toString();
    m_creditCardExpMonth = creditCardExpMonth.toInt();
    m_creditCardExpYear = creditCardExpYear.toInt();
  }

  m_rawJson = json;
  emit changed();


  return true;
}