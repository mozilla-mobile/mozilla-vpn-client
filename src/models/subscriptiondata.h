/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SUBSCRIPTIONDATA_H
#define SUBSCRIPTIONDATA_H

#include <QObject>

class SubscriptionData final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SubscriptionData)

  // Subscription
  Q_PROPERTY(TypeSubscription type MEMBER m_type NOTIFY changed)
  Q_PROPERTY(TypeStatus status MEMBER m_status NOTIFY changed)
  Q_PROPERTY(quint64 createdAt MEMBER m_createdAt NOTIFY changed)
  Q_PROPERTY(quint64 expiresOn MEMBER m_expiresOn NOTIFY changed)
  Q_PROPERTY(bool isCancelled MEMBER m_isCancelled NOTIFY changed)
  Q_PROPERTY(bool isPrivacyBundleSubscriber MEMBER m_isPrivacyBundleSubscriber
                 NOTIFY changed)

  // Plan
  Q_PROPERTY(TypeBillingInterval planBillingInterval MEMBER
                 m_planBillingInterval NOTIFY changed)
  Q_PROPERTY(int planAmount MEMBER m_planAmount NOTIFY changed)
  Q_PROPERTY(QString planCurrency MEMBER m_planCurrency NOTIFY changed)
  Q_PROPERTY(bool planRequiresTax MEMBER m_planRequiresTax NOTIFY changed)

  // Payment
  Q_PROPERTY(QString paymentProvider MEMBER m_paymentProvider NOTIFY changed)
  Q_PROPERTY(QString creditCardBrand MEMBER m_creditCardBrand NOTIFY changed)
  Q_PROPERTY(QString creditCardLast4 MEMBER m_creditCardLast4 NOTIFY changed)
  Q_PROPERTY(int creditCardExpMonth MEMBER m_creditCardExpMonth NOTIFY changed)
  Q_PROPERTY(int creditCardExpYear MEMBER m_creditCardExpYear NOTIFY changed)

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

  void resetData();

  void writeSettings();

 signals:
  void changed();

 private:
  bool fromJsonInternal(const QByteArray& json);

  bool parseSubscriptionDataIap(const QJsonObject& subscriptionData);
  bool parseSubscriptionDataWeb(const QJsonObject& subscriptionData);

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
  bool m_planRequiresTax = false;

  QString m_paymentProvider;
  QString m_creditCardBrand;
  QString m_creditCardLast4;
  int m_creditCardExpMonth = 0;
  int m_creditCardExpYear = 0;
};

#endif  // SUBSCRIPTIONDATA_H
