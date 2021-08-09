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

void AndroidIAPHandler::nativeStartSubscription(Product* product){
    Q_UNUSED(product)}

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

  for (auto value : products) {
    static_cast<AndroidIAPHandler*>(IAPHandler::instance())
        ->updateProductInfo(value);
  }

  AndroidIAPHandler::instance()->productsRegistrationCompleted();
}

void AndroidIAPHandler::updateProductInfo(const QJsonValue& product) {
  Q_ASSERT(m_productsRegistrationState == eRegistering);

  QString productIdentifier = product["sku"].toString();
  Product* productData = findProduct(productIdentifier);
  Q_ASSERT(productData);

  productData->m_price = product["totalPriceString"].toString();
  productData->m_monthlyPrice = product["monthlyPriceString"].toString();
  productData->m_nonLocalizedMonthlyPrice = product["monthlyPrice"].toDouble();
}
