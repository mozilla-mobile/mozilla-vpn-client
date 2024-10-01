/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskpurchase.h"

#include "app.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#ifdef MZ_IOS
#  include "platforms/ios/iosutils.h"
#endif

#include <QJsonObject>

namespace {
Logger logger("TaskPurchase");
}

#ifdef MZ_IOS
// static
TaskPurchase* TaskPurchase::createForIOS(const QString& receipt,
                                         bool isOlderOS) {
  TaskPurchase* task = new TaskPurchase(IOS);
  task->m_iOSData = receipt;
  task->m_isOlderOS = isOlderOS;
  return task;
}
#endif

#ifdef MZ_ANDROID
// static
TaskPurchase* TaskPurchase::createForAndroid(const QString& sku,
                                             const QString& token) {
  TaskPurchase* task = new TaskPurchase(Android);
  task->m_androidSku = sku;
  task->m_androidToken = token;
  return task;
}
#endif

#ifdef MZ_WASM
// static
TaskPurchase* TaskPurchase::createForWasm(const QString& productId) {
  TaskPurchase* task = new TaskPurchase(Wasm);
  task->m_productId = productId;
  return task;
}
#endif

TaskPurchase::TaskPurchase(Op op) : Task("TaskPurchase"), m_op(op) {
  MZ_COUNT_CTOR(TaskPurchase);
}

TaskPurchase::~TaskPurchase() { MZ_COUNT_DTOR(TaskPurchase); }

void TaskPurchase::run() {
#ifdef MZ_IOS
  // APIv2 returns 200, APIv1 returns 201
  int expectedStatusCode;
  if (m_isOlderOS) {
    expectedStatusCode = 201;
  } else {
    expectedStatusCode = 200;
  }
#endif
  NetworkRequest* request = new NetworkRequest(this,
#ifdef MZ_IOS
                                               expectedStatusCode
#else
                                               200
#endif
  );
  request->auth();

  switch (m_op) {
#ifdef MZ_IOS
    case IOS:
      Constants::ApiEndpoint endpoint;
      QJsonObject body;
      if (m_isOlderOS) {
        endpoint = Constants::PurchasesIOS;
        body = QJsonObject{{"receipt", m_iOSData},
                           {"appId", QString::fromNSString(IOSUtils::appId())}};
      } else {
        endpoint = Constants::PurchasesIOSv2;
        body = QJsonObject{{"originalTransactionId", m_iOSData}};
      }
      request->post(Constants::apiUrl(endpoint), body);
      break;
#endif
#ifdef MZ_ANDROID
    case Android:
      request->post(
          Constants::apiUrl(Constants::PurchasesAndroid),
          QJsonObject{{"sku", m_androidSku}, {"token", m_androidToken}});
      break;
#endif
#ifdef MZ_WASM
    case Wasm:
      request->post(Constants::apiUrl(Constants::PurchasesWasm),
                    QJsonObject{{"productId", m_productId}});
      break;
#endif
  }

  Q_ASSERT(request);

  connect(request, &NetworkRequest::requestFailed, this, &TaskPurchase::failed);
  connect(request, &NetworkRequest::requestCompleted, this,
          &TaskPurchase::succeeded);

  // Queued to avoid this task to be deleted before the processing of the slots.
  connect(request, &NetworkRequest::requestFailed, this, &Task::completed,
          Qt::QueuedConnection);
  connect(request, &NetworkRequest::requestCompleted, this, &Task::completed,
          Qt::QueuedConnection);
}
