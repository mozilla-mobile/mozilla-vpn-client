/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskpurchase.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskPurchase");
}

#ifdef MVPN_IOS
// static
TaskPurchase* TaskPurchase::createForIOS(const QString& receipt) {
  TaskPurchase* task = new TaskPurchase(IOS);
  task->m_iOSReceipt = receipt;
  return task;
}
#endif

#ifdef MVPN_ANDROID
// static
TaskPurchase* TaskPurchase::createForAndroid(const QString& sku,
                                             const QString& token) {
  TaskPurchase* task = new TaskPurchase(Android);
  task->m_androidSku = sku;
  task->m_androidToken = token;
  return task;
}
#endif

#ifdef MVPN_WASM
// static
TaskPurchase* TaskPurchase::createForWasm(const QString& productId) {
  TaskPurchase* task = new TaskPurchase(Wasm);
  task->m_productId = productId;
  return task;
}
#endif

TaskPurchase::TaskPurchase(Op op) : Task("TaskPurchase"), m_op(op) {
  MVPN_COUNT_CTOR(TaskPurchase);
}

TaskPurchase::~TaskPurchase() { MVPN_COUNT_DTOR(TaskPurchase); }

void TaskPurchase::run() {
  NetworkRequest* request = nullptr;
  switch (m_op) {
#ifdef MVPN_IOS
    case IOS:
      request = NetworkRequest::createForIOSPurchase(this, m_iOSReceipt);
      break;
#endif
#ifdef MVPN_ANDROID
    case Android:
      request = NetworkRequest::createForAndroidPurchase(this, m_androidSku,
                                                         m_androidToken);
      break;
#endif
#ifdef MVPN_WASM
    case Wasm:
      request = NetworkRequest::createForWasmPurchase(this, m_productId);
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
