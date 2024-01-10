/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjusttasksubmission.h"

#include <QJsonArray>
#include <QJsonObject>

#include "context/constants.h"
#include "logging/logger.h"
#include "networkrequest.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("AdjustTaskSubmission");
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
  MZ_COUNT_CTOR(AdjustTaskSubmission);
}

AdjustTaskSubmission::~AdjustTaskSubmission() {
  MZ_COUNT_DTOR(AdjustTaskSubmission);
}

void AdjustTaskSubmission::run() {
  QJsonObject headersObj;
  for (const QPair<QString, QString>& header : m_headers) {
    headersObj.insert(header.first, header.second);
  }

  QJsonArray unknownParametersArray;
  for (const QString& unknownParameter : m_unknownParameters) {
    unknownParametersArray.append(unknownParameter);
  }

  NetworkRequest* request = new NetworkRequest(this, 200);
  request->post(Constants::apiUrl(Constants::Adjust),
                QJsonObject{{"method", m_method},
                            {"path", m_path},
                            {"headers", headersObj},
                            {"queryParameters", m_queryParameters},
                            {"bodyParameters", m_bodyParameters},
                            {"unknownParameters", unknownParametersArray}});

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
