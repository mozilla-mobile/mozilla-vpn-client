/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusttasksubmission.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_ADJUST, "AdjustTaskSubmission");
}  // namespace

AdjustTaskSubmission::AdjustTaskSubmission(
    const QString& method, const QString& path,
    const QList<QPair<QString, QString>>& headers,
    const QString& queryParameters, const QString& bodyParameters,
    const QStringList& unknownParameters)
    : Task("AdjustTaskSubmission"),
      m_method(method),
      m_path(path),
      m_headers(headers),
      m_queryParameters(queryParameters),
      m_bodyParameters(bodyParameters),
      m_unknownParameters(unknownParameters) {
  MVPN_COUNT_CTOR(AdjustTaskSubmission);
}

AdjustTaskSubmission::~AdjustTaskSubmission() {
  MVPN_COUNT_DTOR(AdjustTaskSubmission);
}

void AdjustTaskSubmission::run() {
  NetworkRequest* request = NetworkRequest::createForAdjustProxy(
      this, m_method, m_path, m_headers, m_queryParameters, m_bodyParameters,
      m_unknownParameters);

  connect(request, &NetworkRequest::requestFailed, this,
          [this, request](QNetworkReply::NetworkError, const QByteArray& data) {
            logger.debug() << "Adjust Proxy request completed with: "
                           << request->statusCode() << ", "
                           << logger.sensitive(data);
            emit operationCompleted(data, request->statusCode());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this, request](const QByteArray& data) {
            logger.debug() << "Adjust Proxy request completed with: "
                           << request->statusCode() << ", "
                           << logger.sensitive(data);
            emit operationCompleted(data, request->statusCode());
            emit completed();
          });
}
