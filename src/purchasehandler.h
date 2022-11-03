/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEHANDLER_H
#define PURCHASEHANDLER_H

#include <QAbstractListModel>
#include <QObject>

class PurchaseHandler : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseHandler)

 public:
  static PurchaseHandler* createInstance();
  static PurchaseHandler* instance();

  Q_INVOKABLE virtual void subscribe(
      const QString& productIdentifier = QString());

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  virtual void subscriptionStarted(const QString& productIdentifier);

 public slots:
  virtual void stopSubscription();
  virtual void startSubscription();

 protected:
  PurchaseHandler(QObject* parent);
  ~PurchaseHandler();

  enum State {
    eActive,
    eInactive,
  } m_subscriptionState = eInactive;
};

#endif  // PURCHASEHANDLER_H
