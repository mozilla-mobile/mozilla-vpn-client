/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidiaphandler.h"
#include "androidutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

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
        {"onBillingNotAvailable", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onBillingNotAvailable)},
        {"onSkuDetailsReceived", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onSkuDetailsReceived)},
        {"onNoPurchases", "()V", reinterpret_cast<void*>(onNoPurchases)},
        {"onPurchaseUpdated", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onPurchaseUpdated)},
        {"onSubscriptionFailed", "()V",
         reinterpret_cast<void*>(onSubscriptionFailed)},
        {"onPurchaseAcknowledged", "()V",
         reinterpret_cast<void*>(onPurchaseAcknowledged)},
        {"onPurchaseAcknowledgeFailed", "()V",
         reinterpret_cast<void*>(onPurchaseAcknowledgeFailed)},
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

void AndroidIAPHandler::launchPlayStore() {
  auto appActivity = QtAndroid::androidActivity();
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "launchPlayStore",
      "(Landroid/app/Activity;)V", appActivity.object());
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
void AndroidIAPHandler::onBillingNotAvailable(JNIEnv* env, jobject thiz,
                                              jstring data) {
  Q_UNUSED(thiz);
  QJsonObject billingResponse =
      AndroidUtils::getQJsonObjectFromJString(env, data);
  logger.info()
      << "onBillingNotAvailable event occured"
      << QJsonDocument(billingResponse).toJson(QJsonDocument::Compact);
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  iap->cancelProductsRegistration();
  emit iap->billingNotAvailable();
}

// static
void AndroidIAPHandler::onSkuDetailsReceived(JNIEnv* env, jobject thiz,
                                             jstring data) {
  Q_UNUSED(thiz);

  QJsonObject obj = AndroidUtils::getQJsonObjectFromJString(env, data);
  if (!obj.contains("products")) {
    logger.error() << "onSkuDetailsReceived - products entry expected.";
    return;
  }
  QJsonArray products = obj["products"].toArray();
  if (products.isEmpty()) {
    logger.error() << "onSkuDetailsRecieved - no products found.";
    return;
  }
  IAPHandler* iap = IAPHandler::instance();
  static_cast<AndroidIAPHandler*>(iap)->updateProductsInfo(products);
  iap->productsRegistrationCompleted();
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
  logger.info() << "onNoPurchases event occured";
}

// static
void AndroidIAPHandler::onSubscriptionFailed(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env)
  Q_UNUSED(thiz);
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  emit iap->subscriptionFailed();
}

// static
void AndroidIAPHandler::onPurchaseUpdated(JNIEnv* env, jobject thiz,
                                          jstring data) {
  Q_UNUSED(thiz);
  QJsonObject json = AndroidUtils::getQJsonObjectFromJString(env, data);
  AndroidUtils::dispatchToMainThread([json] {
    IAPHandler* iap = IAPHandler::instance();
    Q_ASSERT(iap);
    static_cast<AndroidIAPHandler*>(iap)->validatePurchase(json);
  });
}

void AndroidIAPHandler::validatePurchase(QJsonObject json) {
  if (m_subscriptionState != eActive) {
    logger.warning()
        << "onPurchaseUpdated. In a bad state. This is unexpected. Returning.";
    return;
  }

  if (json.isEmpty()) {
    logger.error() << "onPurchaseUpdated, emptyJsonObject received.";
    stopSubscription();
    emit subscriptionFailed();
    return;
  }

  QString sku = json["productId"].toString();
  Product* productData = findProduct(sku);
  Q_ASSERT(productData);
  QString token = json["purchaseToken"].toString();
  Q_ASSERT(!token.isEmpty());

  NetworkRequest* request =
      NetworkRequest::createForAndroidPurchase(this, sku, token);

  connect(
      request, &NetworkRequest::requestFailed,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Purchase validation request to guardian failed";
        MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        stopSubscription();
        emit subscriptionFailed();
        return;
      });

  connect(request, &NetworkRequest::requestCompleted,
          [this, token](const QByteArray& data) {
            logger.debug() << "Products request to guardian completed" << data;

            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(data, &jsonError);

            if (QJsonParseError::NoError != jsonError.error) {
              logger.error()
                  << "onPurchaseUpdated-requestCompleted. Error parsing json. "
                     "Code: "
                  << jsonError.error << "Offset: " << jsonError.offset
                  << "Message: " << jsonError.errorString() << "Data: " << data;
              stopSubscription();
              emit subscriptionFailed();
              return;
            }

            if (!json.isObject() || !json.object().contains("tokenValid")) {
              logger.debug() << "Unexpected json returned";
              stopSubscription();
              emit subscriptionFailed();
              return;
            }

            bool tokenValid = json.object()["tokenValid"].toBool();

            if (!tokenValid) {
              logger.debug() << "tokenValid == false, aborting.";
              stopSubscription();
              emit subscriptionFailed();
              // ToDo - it's not clear what to do in this scenario yet.
              // If we return user to the subscription screen and they
              // try and subscribe again they'll see a "you already have this"
              // message. If they go into that and manually cancel then the
              // purchase can go through.
              return;
            }

            // We can acknowledge the purchase.
            logger.debug() << "tokenValid == true, acknowledging purchase.";
            auto jniString = QAndroidJniObject::fromString(token);
            QAndroidJniObject::callStaticMethod<void>(
                "org/mozilla/firefox/vpn/InAppPurchase", "acknowledgePurchase",
                "(Ljava/lang/String;)V", jniString.object());
          });
}

void AndroidIAPHandler::onPurchaseAcknowledged(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env)
  Q_UNUSED(thiz);
  logger.debug() << "Purchase successfully acknowledged";
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  emit iap->subscriptionCompleted();
}

// static
void AndroidIAPHandler::onPurchaseAcknowledgeFailed(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env)
  Q_UNUSED(thiz);
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  emit iap->subscriptionFailed();
}