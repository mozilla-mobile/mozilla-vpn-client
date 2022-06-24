/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SUBSCRIPTIONDATA_H
#define SUBSCRIPTIONDATA_H

#include <QObject>

class SubscriptionData final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SubscriptionData)

  Q_PROPERTY(bool initialized READ initialized NOTIFY changed)

  // Subscription
  Q_PROPERTY(int createdAt MEMBER m_createdAt CONSTANT)
  Q_PROPERTY(int expiresOn MEMBER m_expiresOn CONSTANT)
  Q_PROPERTY(bool isCancelled MEMBER m_isCancelled CONSTANT)
  Q_PROPERTY(QString status MEMBER m_status CONSTANT)
  Q_PROPERTY(QString type MEMBER m_type CONSTANT)

  // Plan
  Q_PROPERTY(int planAmount MEMBER m_planAmount CONSTANT)
  Q_PROPERTY(QString planCurrency MEMBER m_planCurrency CONSTANT)
  Q_PROPERTY(int planIntervalCount MEMBER m_planIntervalCount CONSTANT)

  // Payment
  Q_PROPERTY(QString paymentProvider MEMBER m_paymentProvider CONSTANT)
  Q_PROPERTY(QString paymentType MEMBER m_paymentType CONSTANT)
  Q_PROPERTY(QString creditCardBrand MEMBER m_creditCardBrand CONSTANT)
  Q_PROPERTY(QString creditCardLast4 MEMBER m_creditCardLast4 CONSTANT)
  Q_PROPERTY(int creditCardExpMonth MEMBER m_creditCardExpMonth CONSTANT)
  Q_PROPERTY(int creditCardExpYear MEMBER m_creditCardExpYear CONSTANT)

 public:
  SubscriptionData();
  ~SubscriptionData();

  [[nodiscard]] bool fromJson(const QByteArray& json);

  bool initialized() const { return !m_rawJson.isEmpty(); }

 signals:
  void changed();

 private:
  bool parseSubscriptionDataIap(const QJsonObject& subscriptionData);
  bool parseSubscriptionDataWeb(const QJsonObject& subscriptionData);

 private:
  QByteArray m_rawJson;

  int m_createdAt = 0;
  int m_expiresOn = 0;
  bool m_isCancelled = false;
  QString m_status;
  QString m_type;

  int m_planAmount = 0;
  QString m_planCurrency;
  int m_planIntervalCount = 0;

  QString m_paymentProvider;
  QString m_paymentType;
  QString m_creditCardBrand;
  QString m_creditCardLast4;
  int m_creditCardExpMonth = 0;
  int m_creditCardExpYear = 0;
};

#endif  // SUBSCRIPTIONDATA_H
