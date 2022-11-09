/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEHANDLER_H
#define PURCHASEHANDLER_H

#include "productshandler.h"

#include <QObject>

class PurchaseHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseHandler)

 public:
  static PurchaseHandler* createInstance();
  static PurchaseHandler* instance();

  Q_INVOKABLE void subscribe(const QString& productIdentifier);
  Q_INVOKABLE void restore();

  void startSubscription(const QString& productIdentifier);

 signals:
  void subscriptionStarted(const QString& productIdentifier);
  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionCompleted();

 public slots:
  void stopSubscription();

 protected:
  PurchaseHandler(QObject* parent);
  ~PurchaseHandler();

  enum State {
    eActive,
    eInactive,
  } m_subscriptionState = eInactive;
};

#endif  // PURCHASEHANDLER_H
