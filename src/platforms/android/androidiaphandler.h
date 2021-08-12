/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDIAPHANDLER_H
#define ANDROIDIAPHANDLER_H

#include "iaphandler.h"
#include "jni.h"

class AndroidIAPHandler final : public IAPHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidIAPHandler)

 public:
  explicit AndroidIAPHandler(QObject* parent);
  ~AndroidIAPHandler();

 public slots:

 protected:
  void nativeRegisterProducts() override;
  void nativeStartSubscription(Product* product) override;

 private:
  QJsonDocument productsToJson();
  void updateProductsInfo(const QJsonArray& products);
  // Functions called via JNI
  static void onSkuDetailsReceived(JNIEnv* env, jobject thiz, jstring data);
  static void onNoPurchases(JNIEnv* env, jobject thiz);
  static void onPurchaseCanceled(JNIEnv* env, jobject thiz);
  static void onPurchaseUpdated(JNIEnv* env, jobject thiz, jstring data);
};

#endif  // DUMMYIAPHANDLER_H
