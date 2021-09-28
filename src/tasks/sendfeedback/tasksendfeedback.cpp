/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksendfeedback.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "networkrequest.h"

constexpr uint32_t FEEDBACK_MESSAGE_MAX_LENGTH = 1000;

constexpr uint32_t FEEDBACK_LOG_MAX_LENGTH = 100000;

namespace {
Logger logger(LOG_MAIN, "TaskSendFeedback");
}

TaskSendFeedback::TaskSendFeedback(const QString& feedbackText,
                                   const QString& logs, const qint8 rating,
                                   const QString& category)
    : Task("TaskSendFeedback"),
      m_feedbackText(feedbackText.left(FEEDBACK_MESSAGE_MAX_LENGTH)),
      m_logs(logs.right(FEEDBACK_LOG_MAX_LENGTH)),
      m_rating(rating),
      m_category(category) {
  MVPN_COUNT_CTOR(TaskSendFeedback);
}

TaskSendFeedback::~TaskSendFeedback() { MVPN_COUNT_DTOR(TaskSendFeedback); }

void TaskSendFeedback::run(MozillaVPN* vpn) {
  Q_UNUSED(vpn);

  logger.debug() << "Sending the feedback";

  NetworkRequest* request = NetworkRequest::createForFeedback(
      this, m_feedbackText, m_logs, m_rating, m_category);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&, int) {
            logger.error() << "Failed to send feedback" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray&) {
            logger.debug() << "Feedback sent";
            emit completed();
          });
}
