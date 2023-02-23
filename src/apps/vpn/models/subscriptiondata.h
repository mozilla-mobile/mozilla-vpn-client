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
  Q_PROPERTY(TypeSubscription type MEMBER m_type CONSTANT)
  Q_PROPERTY(TypeStatus status MEMBER m_status CONSTANT)
  Q_PROPERTY(quint64 createdAt MEMBER m_createdAt CONSTANT)
  Q_PROPERTY(quint64 expiresOn MEMBER m_expiresOn CONSTANT)
  Q_PROPERTY(bool isCancelled MEMBER m_isCancelled CONSTANT)
  Q_PROPERTY(bool isPrivacyBundleSubscriber MEMBER m_isPrivacyBundleSubscriber
                 CONSTANT)

  // Plan
  Q_PROPERTY(TypeBillingInterval planBillingInterval MEMBER
                 m_planBillingInterval CONSTANT)
  Q_PROPERTY(int planAmount MEMBER m_planAmount CONSTANT)
  Q_PROPERTY(QString planCurrency MEMBER m_planCurrency CONSTANT)
  Q_PROPERTY(bool planRequiresTax MEMBER m_planRequiresTax CONSTANT)

  // Payment
  Q_PROPERTY(QString paymentProvider MEMBER m_paymentProvider CONSTANT)
  Q_PROPERTY(QString creditCardBrand MEMBER m_creditCardBrand CONSTANT)
  Q_PROPERTY(QString creditCardLast4 MEMBER m_creditCardLast4 CONSTANT)
  Q_PROPERTY(int creditCardExpMonth MEMBER m_creditCardExpMonth CONSTANT)
  Q_PROPERTY(int creditCardExpYear MEMBER m_creditCardExpYear CONSTANT)

  Q_PROPERTY(bool plusTax READ plusTax NOTIFY plusTaxChanged)

 public:
  SubscriptionData();
  ~SubscriptionData();

  enum TypeSubscription {
    SubscriptionApple,
    SubscriptionGoogle,
    SubscriptionWeb,
    SubscriptionUnknown,
  };
  Q_ENUM(TypeSubscription)

  enum TypeStatus {
    Active,
    Inactive,
  };
  Q_ENUM(TypeStatus)

  enum TypeBillingInterval {
    BillingIntervalMonthly,
    BillingIntervalHalfYearly,
    BillingIntervalYearly,
    BillingIntervalUnknown,
  };
  Q_ENUM(TypeBillingInterval)

  [[nodiscard]] bool fromJson(const QByteArray& json);

  [[nodiscard]] bool fromSettings();

  bool initialized() const { return !m_rawJson.isEmpty(); }

  void reset() { m_rawJson.clear(); }

  void writeSettings();

  bool plusTax();

 signals:
  void changed();
  void plusTaxChanged();

 private:
  bool fromJsonInternal(const QByteArray& json);

  bool parseSubscriptionDataIap(const QJsonObject& subscriptionData);
  bool parseSubscriptionDataWeb(const QJsonObject& subscriptionData);
  void resetData();

 private:
  QByteArray m_rawJson;

  TypeSubscription m_type = SubscriptionUnknown;
  TypeStatus m_status = Inactive;
  quint64 m_createdAt = 0;
  quint64 m_expiresOn = 0;
  bool m_isCancelled = false;
  bool m_isPrivacyBundleSubscriber = false;

  TypeBillingInterval m_planBillingInterval = BillingIntervalUnknown;
  int m_planAmount = 0;
  QString m_planCurrency;
  bool m_planRequiresTax;

  QString m_paymentProvider;
  QString m_creditCardBrand;
  QString m_creditCardLast4;
  int m_creditCardExpMonth = 0;
  int m_creditCardExpYear = 0;
};

#endif  // SUBSCRIPTIONDATA_H
