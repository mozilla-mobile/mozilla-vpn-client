/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKPURCHASE_H
#define TASKPURCHASE_H

#include "task.h"

#include <QObject>
#include <QNetworkReply>

class TaskPurchase final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskPurchase)

 public:
#ifdef MVPN_IOS
  static TaskPurchase* createForIOS(const QString& receipt);
#endif
#ifdef MVPN_ANDROID
  static TaskPurchase* createForAndroid(const QString& sku,
                                        const QString& token);
#endif
#ifdef MVPN_WASM
  static TaskPurchase* createForWasm(const QString& productId);
#endif

  ~TaskPurchase();

  void run() override;

 signals:
  void failed(QNetworkReply::NetworkError error, const QByteArray& data);
  void succeeded(const QByteArray& data);

 private:
  enum Op {
#ifdef MVPN_IOS
    IOS,
#endif
#ifdef MVPN_ANDROID
    Android,
#endif
#ifdef MVPN_WASM
    Wasm,
#endif
  };

  explicit TaskPurchase(Op op);

 private:
  Op m_op;
#ifdef MVPN_IOS
  QString m_iOSReceipt;
#endif
#ifdef MVPN_ANDROID
  QString m_androidSku;
  QString m_androidToken;
#endif
#ifdef MVPN_WASM
  QString m_productId;
#endif
};

#endif  // TASKPURCHASE_H
