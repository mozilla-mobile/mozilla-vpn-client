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
  void launchPlayStore();

 protected:
  void nativeRegisterProducts() override;
  void nativeStartSubscription(ProductsHandler::Product* product) override;
  void nativeRestoreSubscription() override;

 private:
  void maybeInit();
  void updateProductsInfo(const QJsonArray& products);
  void processPurchase(QJsonObject purchase);
  void validatePurchase(QJsonObject purchase);

  // Functions called via JNI
  // Successes
  static void onPurchaseAcknowledged(JNIEnv* env, jobject thiz);
  static void onPurchaseUpdated(JNIEnv* env, jobject thiz, jstring data);
  static void onSkuDetailsReceived(JNIEnv* env, jobject thiz, jstring data);
  // Failures
  static void onBillingNotAvailable(JNIEnv* env, jobject thiz, jstring data);
  static void onPurchaseAcknowledgeFailed(JNIEnv* env, jobject thiz,
                                          jstring data);
  static void onSkuDetailsFailed(JNIEnv* env, jobject thiz, jstring data);
  static void onSubscriptionFailed(JNIEnv* env, jobject thiz, jstring data);

 private:
  bool m_init = false;
};

#endif  // ANDROIDIAPHANDLER_H
