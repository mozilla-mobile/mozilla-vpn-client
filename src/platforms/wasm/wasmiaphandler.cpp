/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/wasm/wasmiaphandler.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/purchase/taskpurchase.h"
#include "taskscheduler.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>
#include <QTimer>

namespace {
Logger logger(LOG_IAP, "WasmIAPHandler");
}  // namespace

WasmIAPHandler::WasmIAPHandler(QObject* parent) : IAPHandler(parent) {
  MVPN_COUNT_CTOR(WasmIAPHandler);
}

WasmIAPHandler::~WasmIAPHandler() { MVPN_COUNT_DTOR(WasmIAPHandler); }

void WasmIAPHandler::nativeRegisterProducts() {
  // Let's use the trialDays to sort the products in the wasm client
  int trialDays = 100;
  for (Product& product : m_products) {
    product.m_price = QString("%1 Dupondius")
                          .arg(QRandomGenerator::system()->bounded(1, 100));
    product.m_monthlyPrice =
        QString("%1 Sestertius")
            .arg(QRandomGenerator::system()->bounded(1, 100));
    product.m_trialDays = trialDays--;
    product.m_nonLocalizedMonthlyPrice = 123;
  }

  QTimer::singleShot(200, this,
                     [this]() { emit productsRegistrationCompleted(); });
}

void WasmIAPHandler::nativeStartSubscription(Product* product) {
  TaskPurchase* purchaseTask = TaskPurchase::createForWasm(product->m_name);
  Q_ASSERT(purchaseTask);

  connect(
      purchaseTask, &TaskPurchase::failed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Purchase validation request to guardian failed";
        MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        stopSubscription();
        emit subscriptionNotValidated();
      });

  connect(purchaseTask, &TaskPurchase::succeeded, this,
          [this](const QByteArray& data) {
            logger.debug() << "Products request to guardian completed" << data;

            QJsonDocument json = QJsonDocument::fromJson(data);
            QJsonObject obj = json.object();
            QString status = obj["status"].toString();

            if (status == "invalid") {
              stopSubscription();
              emit subscriptionNotValidated();
              return;
            }

            if (status == "failed") {
              stopSubscription();
              emit subscriptionFailed();
              return;
            }

            if (status == "canceled") {
              stopSubscription();
              emit subscriptionCanceled();
              return;
            }

            if (status == "already-subscribed") {
              stopSubscription();
              emit alreadySubscribed();
              return;
            }

            if (status == "billing-not-available") {
              stopSubscription();
              emit billingNotAvailable();
              return;
            }

            if (status == "not-validated") {
              stopSubscription();
              emit subscriptionNotValidated();
              return;
            }

            stopSubscription();
            emit subscriptionCompleted();
          });

  TaskScheduler::scheduleTask(purchaseTask);
}

void WasmIAPHandler::nativeRestoreSubscription() {
  logger.error() << "Restore not possible on Wasm (yet?)!!!";
}
