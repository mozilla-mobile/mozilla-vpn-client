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
  Q_PROPERTY(int createdAt READ createdAt CONSTANT)
  Q_PROPERTY(int expiresOn READ expiresOn CONSTANT)
  Q_PROPERTY(bool isCancelled READ isCancelled CONSTANT)
  Q_PROPERTY(QString status READ status CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)

  // Plan
  Q_PROPERTY(int planAmount READ planAmount CONSTANT)
  Q_PROPERTY(QString planCurrency READ planCurrency CONSTANT)
  Q_PROPERTY(int planIntervalCount READ planIntervalCount CONSTANT)

  // Payment
  Q_PROPERTY(QString paymentType READ paymentType CONSTANT)
  Q_PROPERTY(QString creditCardBrand READ creditCardBrand CONSTANT)
  Q_PROPERTY(QString creditCardLast4 READ creditCardLast4 CONSTANT)
  Q_PROPERTY(int creditCardExpMonth READ creditCardExpMonth CONSTANT)
  Q_PROPERTY(int creditCardExpYear READ creditCardExpYear CONSTANT)

 public:
  SubscriptionData();
  ~SubscriptionData();

  // [[nodiscard]] bool fromJson(const QByteArray& json);
  Q_INVOKABLE bool fromJson(const QByteArray& json);

  bool initialized() const { return !m_rawJson.isEmpty(); }

  int createdAt() const { return m_createdAt; }
  int expiresOn() const { return m_expiresOn; }
  bool isCancelled() const { return m_isCancelled; }
  QString status() const { return m_status; }
  QString type() const { return m_type; }

  int planAmount() const { return m_planAmount; }
  QString planCurrency() const { return m_planCurrency; }
  int planIntervalCount() const { return m_planIntervalCount; }

  QString paymentType() const { return m_paymentType; }
  QString creditCardBrand() const { return m_creditCardBrand; }
  QString creditCardLast4() const { return m_creditCardLast4; }
  int creditCardExpMonth() const { return m_creditCardExpMonth; }
  int creditCardExpYear() const { return m_creditCardExpYear; }

 signals:
  void changed();

 private:
  QByteArray m_rawJson;

  int m_createdAt;
  int m_expiresOn;
  bool m_isCancelled;
  QString m_status;
  QString m_type;

  int m_planAmount;
  QString m_planCurrency;
  int m_planIntervalCount;

  QString m_paymentType;
  QString m_creditCardBrand;
  QString m_creditCardLast4;
  int m_creditCardExpMonth;
  int m_creditCardExpYear;
};

#endif  // SUBCRIPTIONDATA_H
