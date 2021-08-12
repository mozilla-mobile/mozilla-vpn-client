/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/android/androidiaphandler.h"

#include "leakdetector.h"
#include "logger.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtAndroid>

namespace {
Logger logger(LOG_IAP, "AndroidIAPHandler");
constexpr auto CLASSNAME = "org.mozilla.firefox.vpn.InAppPurchase";
}  // namespace

AndroidIAPHandler::AndroidIAPHandler(QObject* parent) : IAPHandler(parent) {
  MVPN_COUNT_CTOR(AndroidIAPHandler);

  // Init the billing client
  auto appContext = QtAndroid::androidActivity().callObjectMethod(
      "getApplicationContext", "()Landroid/content/Context;");
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "init",
      "(Landroid/content/Context;)V", appContext.object());

  // Hook together implementations for functions called by native code
  QtAndroid::runOnAndroidThreadSync([]() {
    JNINativeMethod methods[]{
        {"onSkuDetailsReceived", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onSkuDetailsReceived)},
        {"onNoPurchases", "()V", reinterpret_cast<void*>(onNoPurchases)},
        {"onPurchaseCanceled", "()V",
         reinterpret_cast<void*>(onPurchaseCanceled)},
        {"onPurchaseUpdated", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onPurchaseUpdated)},
    };
    QAndroidJniObject javaClass(CLASSNAME);
    QAndroidJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass, methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
  });
}

AndroidIAPHandler::~AndroidIAPHandler() {
  MVPN_COUNT_DTOR(AndroidIAPHandler);
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "deinit", "()V");
}

void AndroidIAPHandler::nativeRegisterProducts() {
  QJsonDocument productData = productsToJson();
  auto jniString =
      QAndroidJniObject::fromString(productData.toJson(QJsonDocument::Compact));

  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "lookupProductsInPlayStore",
      "(Ljava/lang/String;)V", jniString.object());
}

QJsonDocument AndroidIAPHandler::productsToJson() {
  QJsonArray jsonProducts;
  for (auto p : m_products) {
    QJsonObject jsonProduct;
    jsonProduct["id"] = p.m_name;
    jsonProduct["monthCount"] =
        QVariant::fromValue(productTypeToMonthCount(p.m_type)).toInt();
    jsonProducts.append(jsonProduct);
  }
  QJsonObject root;
  root.insert("products", jsonProducts);
  return QJsonDocument(root);
}

// static
void AndroidIAPHandler::onSkuDetailsReceived(JNIEnv* env, jobject thiz,
                                             jstring data) {
  Q_UNUSED(thiz);

  const char* buffer = env->GetStringUTFChars(data, nullptr);
  if (!buffer) {
    logger.error() << "onSkuDetailsReceived - failed to parse data.";
    return;
  }
  QByteArray raw = QByteArray(buffer);
  env->ReleaseStringUTFChars(data, buffer);

  logger.debug() << "onSkuDetailsReceived - parsing raw data: " << raw;

  QJsonParseError jsonError;
  QJsonDocument json = QJsonDocument::fromJson(raw, &jsonError);

  if (QJsonParseError::NoError != jsonError.error) {
    logger.error() << "onSkuDetailsRecieved, Error parsing json. Code: "
                   << jsonError.error << "Offset: " << jsonError.offset
                   << "Message: " << jsonError.errorString() << "Data: " << raw;
    return;
  }

  if (!json.isObject()) {
    logger.error() << "onSkuDetailsReceived - object expected.";
    return;
  }

  QJsonObject obj = json.object();
  if (!obj.contains("products")) {
    logger.error() << "onSkuDetailsReceived - products entry expected.";
    return;
  }

  QJsonArray products = obj["products"].toArray();
  if (products.isEmpty()) {
    logger.error() << "onSkuDetailsRecieved - no products found.";
    return;
  }

  static_cast<AndroidIAPHandler*>(IAPHandler::instance())
      ->updateProductsInfo(products);
  AndroidIAPHandler::instance()->productsRegistrationCompleted();
}

void AndroidIAPHandler::updateProductsInfo(const QJsonArray& returnedProducts) {
  Q_ASSERT(m_productsRegistrationState == eRegistering);

  QList<QString> productsUpdated;
  for (auto product : returnedProducts) {
    QString productIdentifier = product["sku"].toString();
    Product* productData = findProduct(productIdentifier);
    Q_ASSERT(productData);

    productData->m_price = product["totalPriceString"].toString();
    productData->m_monthlyPrice = product["monthlyPriceString"].toString();
    productData->m_nonLocalizedMonthlyPrice =
        product["monthlyPrice"].toDouble();

    productsUpdated.append(productIdentifier);
  }
  // Remove products from m_products if we didn't get info back from google
  // about them.
  for (auto product : m_products) {
    if (!productsUpdated.contains(product.m_name)) {
      unknownProductRegistered(product.m_name);
    }
  }
}

void AndroidIAPHandler::nativeStartSubscription(Product* product) {
  auto jniString = QAndroidJniObject::fromString(product->m_name);
  auto appActivity = QtAndroid::androidActivity();

  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "purchaseProduct",
      "(Ljava/lang/String;Landroid/app/Activity;)V", jniString.object(),
      appActivity.object());
}

// static
void AndroidIAPHandler::onNoPurchases(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env)
  Q_UNUSED(thiz);
  // ToDo - I'm not sure when this scenario would occur
  // and so I'm not sure what the best way to handle it is.
}

// static
void AndroidIAPHandler::onPurchaseCanceled(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env)
  Q_UNUSED(thiz);
  IAPHandler::instance()->stopSubscription();
  IAPHandler::instance()->subscriptionFailed();
  IAPHandler::instance()->productsRegistered();
}

// static
void AndroidIAPHandler::onPurchaseUpdated(JNIEnv* env, jobject thiz,
                                          jstring data) {
  Q_UNUSED(thiz);

  const char* buffer = env->GetStringUTFChars(data, nullptr);
  if (!buffer) {
    logger.error() << "purchaseUpdated - failed to parse data.";
    return;
  }
  QByteArray raw = QByteArray(buffer);
  env->ReleaseStringUTFChars(data, buffer);

  logger.debug() << "purchaseUpdated - parsing raw data: " << raw;

  QJsonParseError jsonError;
  QJsonDocument json = QJsonDocument::fromJson(raw, &jsonError);

  if (QJsonParseError::NoError != jsonError.error) {
    logger.error() << "purchaseUpdated, Error parsing json. Code: "
                   << jsonError.error << "Offset: " << jsonError.offset
                   << "Message: " << jsonError.errorString() << "Data: " << raw;
    return;
  }

  IAPHandler::instance()->stopSubscription();
  // NEXT UP - make this pass through correctly and
  // then fill in processCompletedTransactions
  // IAPHandler::instance()->processCompletedTransactions(json);
}
