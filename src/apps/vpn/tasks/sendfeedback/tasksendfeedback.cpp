/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksendfeedback.h"

#include <QJsonObject>

#include "appconstants.h"
#include "env.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkmanager.h"
#include "networkrequest.h"

constexpr uint32_t FEEDBACK_MESSAGE_MAX_LENGTH = 1000;

constexpr uint32_t FEEDBACK_LOG_MAX_LENGTH = 100000;

namespace {
Logger logger("TaskSendFeedback");
}

TaskSendFeedback::TaskSendFeedback(const QString& feedbackText,
                                   const QString& logs, const qint8 rating,
                                   const QString& category)
    : Task("TaskSendFeedback"),
      m_feedbackText(feedbackText.left(FEEDBACK_MESSAGE_MAX_LENGTH)),
      m_logs(logs.right(FEEDBACK_LOG_MAX_LENGTH)),
      m_rating(rating),
      m_category(category) {
  MZ_COUNT_CTOR(TaskSendFeedback);
}

TaskSendFeedback::~TaskSendFeedback() { MZ_COUNT_DTOR(TaskSendFeedback); }

void TaskSendFeedback::run() {
  logger.debug() << "Sending the feedback";

  NetworkRequest* request = new NetworkRequest(this, 201);
  request->auth(MozillaVPN::authorizationHeader());
  request->post(
      AppConstants::apiUrl(AppConstants::Feedback),
      QJsonObject{{"feedbackText", m_feedbackText},
                  {"logs", m_logs},
                  {"versionString", Env::versionString()},
                  {"platformVersion", QString(NetworkManager::osVersion())},
                  {"rating", m_rating},
                  {"category", m_category}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to send feedback" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray&) {
            logger.debug() << "Feedback sent";
            emit completed();
          });
}
