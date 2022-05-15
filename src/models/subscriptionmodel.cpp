/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "subscriptionmodel.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_MODEL, "SubscriptionModel");
}

SubscriptionModel::SubscriptionModel() { MVPN_COUNT_CTOR(SubscriptionModel); }

SubscriptionModel::~SubscriptionModel() { MVPN_COUNT_DTOR(SubscriptionModel); }

bool SubscriptionModel::fromJson(const QByteArray& json) {
  if (!json.isEmpty() && m_rawJson == json) {
    logger.debug() << "Data has not changed";
    return true;
  }

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }

  // Subscription data
  QJsonObject obj = doc.object();
  int subCreatedAt = obj.value("created_at").toInt();
  int subExpiresOn = obj.value("expires_on").toInt();
  bool subIsCancelled = obj.value("is_cancelled").toBool();
  QString subStatus = obj.value("status").toString();
  QString subType = obj.value("type").toString();
  // Payment
  QJsonObject paymentData = obj.value("payment").toObject();
  QString creditCardBrand = paymentData.value("credit_card_brand").toString();
  int creditCardExpMonth = paymentData.value("credit_card_exp_month").toInt();
  int creditCardExpYear = paymentData.value("credit_card_exp_year").toInt();
  QString creditCardLast4 = paymentData.value("credit_card_last4").toString();
  QString paymentType = paymentData.value("type").toString();
  // Plan
  QJsonObject planData = obj.value("plan").toObject();
  int planAmount = planData.value("amount").toInt();
  QString planCurrency = planData.value("currency").toString();
  int planIntervalCount = planData.value("interval_count").toInt();

  // Populate subscription model
  beginResetModel();
  m_subscriptionList.clear();

  m_subscriptionList.append(SubscriptionItem{"plan", { planCurrency, QString::number(planAmount), QString::number(planIntervalCount) }});
  m_subscriptionList.append(SubscriptionItem{"status", { subStatus }});
  m_subscriptionList.append(SubscriptionItem{"activated", { QString::number(subCreatedAt)} });

  // Subscription already is or will expire?
  if (subIsCancelled || subStatus == "inactive") {
    m_subscriptionList.append(SubscriptionItem{"expires", { QString::number(subExpiresOn) }});  
  } else {
    m_subscriptionList.append(SubscriptionItem{"next-billed", { QString::number(subExpiresOn) }});
  }

  // Payment is not handled via Google or Apple?
  if (subType == "web" && paymentType == "credit") {
    m_subscriptionList.append(SubscriptionItem{"payment-method", { paymentType, creditCardBrand, creditCardLast4 }});
    m_subscriptionList.append(SubscriptionItem{"payment-expires", { QString::number(creditCardExpMonth), QString::number(creditCardExpYear) }});
  } else {
    m_subscriptionList.append(SubscriptionItem{"payment-method", { paymentType }});
  }
  endResetModel();

  m_rawJson = json;
  emit changed();

  return true;
}

QHash<int, QByteArray> SubscriptionModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[KeyRole] = "key";
  roles[ValuesRole] = "values";
  return roles;
}

int SubscriptionModel::rowCount(const QModelIndex&) const {
  return m_subscriptionList.count();
}

QVariant SubscriptionModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case KeyRole:
      return QVariant(m_subscriptionList.at(index.row()).m_key);

    case ValuesRole:
      return QVariant(m_subscriptionList.at(index.row()).m_values);

    default:
      return QVariant();
  }
}
