/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksentry.h"

#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskSentry");
}

TaskSentry::TaskSentry(const QByteArray& envelope) : Task("TaskSentry") {
  MZ_COUNT_CTOR(TaskSentry);
  m_envelope = envelope;
}

TaskSentry::~TaskSentry() { MZ_COUNT_DTOR(TaskSentry); }

void TaskSentry::run() {
  NetworkRequest* request = NetworkRequest::createForSentry(this, m_envelope);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            Q_UNUSED(error);
            logger.error() << "Failed to send envelope";
            emit completed();
          });
  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            Q_UNUSED(data);
            logger.debug() << "Sentry sent events";
            emit completed();
          });
}
