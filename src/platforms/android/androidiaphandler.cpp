/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidiaphandler.h"
#include "androidutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "tasks/purchase/taskpurchase.h"
#include "taskscheduler.h"

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
  maybeInit();
}

AndroidIAPHandler::~AndroidIAPHandler() {
  MVPN_COUNT_DTOR(AndroidIAPHandler);
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "deinit", "()V");
}

void AndroidIAPHandler::maybeInit() {
  if (m_init) {
    return;
  }
  // Init the billing client
  auto appContext = QtAndroid::androidActivity().callObjectMethod(
      "getApplicationContext", "()Landroid/content/Context;");
  if (!appContext.isValid()) {
    // This is a race condition, we could be here while android has not finished
    // activity::onCreate on the Ui thread. In this case the context is null.
    logger.debug() << "Android IAP handler init skipped";
    return;
  }
  logger.debug() << "Android IAP handler init";
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "init",
      "(Landroid/content/Context;)V", appContext.object());

  // Hook together implementations for functions called by native code
  QtAndroid::runOnAndroidThreadSync([]() {
    JNINativeMethod methods[]{
        // Failures
        {"onBillingNotAvailable", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onBillingNotAvailable)},
        {"onPurchaseAcknowledgeFailed", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onPurchaseAcknowledgeFailed)},
        {"onSkuDetailsFailed", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onSkuDetailsFailed)},
        {"onSubscriptionFailed", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onSubscriptionFailed)},
        // Successes
        {"onPurchaseAcknowledged", "()V",
         reinterpret_cast<void*>(onPurchaseAcknowledged)},
        {"onPurchaseUpdated", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onPurchaseUpdated)},
        {"onSkuDetailsReceived", "(Ljava/lang/String;)V",
         reinterpret_cast<void*>(onSkuDetailsReceived)},
    };
    QAndroidJniEnvironment env;
    jclass objectClass = env.findClass(CLASSNAME);
    if (objectClass == nullptr) {
      logger.error() << "Android-IAP Class is Null?!";
      return;
    }
    env->RegisterNatives(objectClass, methods,
                         sizeof(methods) / sizeof(methods[0]));
  });
  m_init = true;
}

void AndroidIAPHandler::nativeRegisterProducts() {
  maybeInit();
  Q_ASSERT(m_init);
  // Convert products to JSON
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
  QJsonDocument productData = QJsonDocument(root);
  auto jniString =
      QAndroidJniObject::fromString(productData.toJson(QJsonDocument::Compact));

  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "lookupProductsInPlayStore",
      "(Ljava/lang/String;)V", jniString.object());
}

void AndroidIAPHandler::nativeStartSubscription(Product* product) {
  maybeInit();
  Q_ASSERT(m_init);
  auto jniString = QAndroidJniObject::fromString(product->m_name);
  auto appActivity = QtAndroid::androidActivity();
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "purchaseProduct",
      "(Ljava/lang/String;Landroid/app/Activity;)V", jniString.object(),
      appActivity.object());
}

void AndroidIAPHandler::launchPlayStore() {
  maybeInit();
  Q_ASSERT(m_init);
  auto appActivity = QtAndroid::androidActivity();
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/InAppPurchase", "launchPlayStore",
      "(Landroid/app/Activity;)V", appActivity.object());
}

// Call backs from JNI - Successes

// static
void AndroidIAPHandler::onPurchaseAcknowledged(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env)
  Q_UNUSED(thiz);
  logger.debug() << "Purchase successfully acknowledged";
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  emit iap->subscriptionCompleted();
}

// static
void AndroidIAPHandler::onPurchaseUpdated(JNIEnv* env, jobject thiz,
                                          jstring data) {
  /**
   * This function may be called whenever we receive information
   * about a purchase. That should be in two scenarios:
   * - after running a queryPurchases at the same time as skuDetails
   *   in order to see whether we have an existing subscription but
   *   a different FxA account
   * - after initiating a subscription, from which we then need to
   *   validate that subscription for acknowledgement.
   * Note, it doesn't happen after acknowledging.
   */
  Q_UNUSED(thiz);

  QJsonObject purchase = AndroidUtils::getQJsonObjectFromJString(env, data);
  Q_ASSERT(!purchase.isEmpty());
  logger.debug() << "Got purchase info"
                 << logger.sensitive(QJsonDocument(purchase).toJson());

  AndroidUtils::dispatchToMainThread([purchase] {
    IAPHandler* iap = IAPHandler::instance();
    Q_ASSERT(iap);
    static_cast<AndroidIAPHandler*>(iap)->processPurchase(purchase);
  });
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
  IAPHandler* iap = IAPHandler::instance();
  if (products.isEmpty()) {
    logger.error() << "onSkuDetailsRecieved - no products found.";
    iap->stopProductsRegistration();
    return;
  }
  static_cast<AndroidIAPHandler*>(iap)->updateProductsInfo(products);
  iap->productsRegistrationCompleted();
}

// Call backs from JNI - Failures

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
  if (billingResponse["code"].toInt() == -99) {
    // The billing service was disconnected.
    // Lets try a reset if we need a subscription.
    // TODO - This is speculative. I put it here because I got an inappropriate
    // launch of the "Sign in to Play Store" window. But I'm not exactly sure
    // how to trigger a billing service disconnected and I'm not sure what the
    // right action should be.
    auto& vpn = MozillaVPN::instance();
    if (vpn.user()->subscriptionNeeded()) {
      vpn.reset(true);
      return;
    }
  }
  iap->stopSubscription();
  iap->stopProductsRegistration();
  emit iap->billingNotAvailable();
}

// static
void AndroidIAPHandler::onPurchaseAcknowledgeFailed(JNIEnv* env, jobject thiz,
                                                    jstring data) {
  Q_UNUSED(thiz);
  QJsonObject json = AndroidUtils::getQJsonObjectFromJString(env, data);
  logger.error() << "onPurchaseAcknowledgeFailed"
                 << QJsonDocument(json).toJson(QJsonDocument::Compact);
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  emit iap->subscriptionNotValidated();
}

// static
void AndroidIAPHandler::onSkuDetailsFailed(JNIEnv* env, jobject thiz,
                                           jstring data) {
  Q_UNUSED(thiz);
  QJsonObject json = AndroidUtils::getQJsonObjectFromJString(env, data);
  logger.error() << "onSkuDetailsFailed"
                 << QJsonDocument(json).toJson(QJsonDocument::Compact);
  IAPHandler* iap = IAPHandler::instance();
  iap->stopProductsRegistration();
}

// static
void AndroidIAPHandler::onSubscriptionFailed(JNIEnv* env, jobject thiz,
                                             jstring data) {
  Q_UNUSED(thiz);
  QJsonObject json = AndroidUtils::getQJsonObjectFromJString(env, data);
  logger.error() << "onSubscriptionFailed"
                 << QJsonDocument(json).toJson(QJsonDocument::Compact);
  IAPHandler* iap = IAPHandler::instance();
  iap->stopSubscription();
  emit iap->subscriptionFailed();
}

// The rest - instance methods

void AndroidIAPHandler::updateProductsInfo(const QJsonArray& returnedProducts) {
  Q_ASSERT(m_productsRegistrationState == eRegistering);

  QStringList productsUpdated;
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

void AndroidIAPHandler::processPurchase(QJsonObject purchase) {
  // If we're trying to use IAP, but have a valid subscription,
  // we're already subscribed and need to throw up a blocker.
  bool purchaseAcknowledged = purchase["acknowledged"].toBool();
  bool autoRenewing = purchase["autoRenewing"].toBool();
  bool cancelled = !autoRenewing;

  // We need to validate / acknowledge an unAcknowledged purchase
  // that hasn't been cancelled.
  if (!purchaseAcknowledged & !cancelled) {
    validatePurchase(purchase);
  }

  if (purchaseAcknowledged &&
      MozillaVPN::instance().user()->subscriptionNeeded()) {
    logger.info() << "User is listed as subscriptionNeeded, but we have an "
                     "acknowledgedPurchase";
    stopSubscription();
    emit alreadySubscribed();
  }

  // Otherwise this is a no-op.
}

void AndroidIAPHandler::validatePurchase(QJsonObject purchase) {
  QString sku = purchase["productId"].toString();
  Product* productData = findProduct(sku);
  Q_ASSERT(productData);
  QString token = purchase["purchaseToken"].toString();
  Q_ASSERT(!token.isEmpty());

  TaskPurchase* purchaseTask = TaskPurchase::createForAndroid(sku, token);
  Q_ASSERT(purchaseTask);

  connect(
      purchaseTask, &TaskPurchase::failed,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Purchase validation request to guardian failed";
        MozillaVPN::instance().errorHandle(ErrorHandler::toErrorType(error));
        stopSubscription();
        emit subscriptionNotValidated();
      });

  connect(purchaseTask, &TaskPurchase::succeeded,
          [this, token](const QByteArray& data) {
            logger.debug() << "Products request to guardian completed" << data;

            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(data, &jsonError);

            if (QJsonParseError::NoError != jsonError.error) {
              logger.error()
                  << "onPurchaseUpdated-requestCompleted. Error parsing json. "
                     "Code: "
                  << jsonError.error << "Offset: " << jsonError.offset
                  << "Message: " << jsonError.errorString();
              stopSubscription();
              emit subscriptionNotValidated();
              return;
            }

            if (!json.isObject() || !json.object().contains("tokenValid")) {
              logger.error() << "Unexpected json returned";
              stopSubscription();
              emit subscriptionNotValidated();
              return;
            }

            bool tokenValid = json.object()["tokenValid"].toBool();

            if (!tokenValid) {
              logger.info() << "tokenValid == false, aborting.";
              stopSubscription();
              emit subscriptionNotValidated();
              return;
            }

            // We can acknowledge the purchase.
            logger.info() << "tokenValid == true, acknowledging purchase.";
            auto jniString = QAndroidJniObject::fromString(token);
            QAndroidJniObject::callStaticMethod<void>(
                "org/mozilla/firefox/vpn/InAppPurchase", "acknowledgePurchase",
                "(Ljava/lang/String;)V", jniString.object());
          });

  TaskScheduler::scheduleTask(purchaseTask);
}
