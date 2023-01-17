/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskpurchase.h"

#include "appconstants.h"
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
TaskPurchase* TaskPurchase::createForIOS(const QString& receipt) {
  TaskPurchase* task = new TaskPurchase(IOS);
  task->m_iOSReceipt = receipt;
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
  NetworkRequest* request = new NetworkRequest(this, 200);

  switch (m_op) {
#ifdef MZ_IOS
    case IOS:
      request->post(
          AppConstants::apiUrl(AppConstants::PurchasesIOS),
          QJsonObject{{"receipt", m_iOSReceipt},
                      {"appId", QString::fromNSString(IOSUtils::appId())}});
      break;
#endif
#ifdef MZ_ANDROID
    case Android:
      request->post(
          AppConstants::apiUrl(AppConstants::PurchasesAndroid),
          QJsonObject{{"sku", m_androidSku}, {"token", m_androidToken}});
      break;
#endif
#ifdef MZ_WASM
    case Wasm:
      request->post(AppConstants::apiUrl(AppConstants::PurchasesWasm),
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
