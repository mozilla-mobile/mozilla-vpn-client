/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcreatesupportticket.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

constexpr uint32_t SUPPORT_TICKET_SUBJECT_MAX_LENGTH = 300;

constexpr uint32_t SUPPORT_TICKET_MESSAGE_MAX_LENGTH = 1000;

constexpr uint32_t SUPPORT_TICKET_LOG_MAX_LENGTH = 100000;

namespace {
Logger logger(LOG_MAIN, "TaskCreateSupportTicket");
}

TaskCreateSupportTicket::TaskCreateSupportTicket(const QString& email,
                                                 const QString& subject,
                                                 const QString& issueText,
                                                 const QString& logs,
                                                 const QString& category)
    : Task("TaskCreateSupportTicket"),
      m_email(email),
      m_subject(subject.left(SUPPORT_TICKET_SUBJECT_MAX_LENGTH)),
      m_issueText(issueText.left(SUPPORT_TICKET_MESSAGE_MAX_LENGTH)),
      m_logs(logs.right(SUPPORT_TICKET_LOG_MAX_LENGTH)),
      m_category(category) {
  MVPN_COUNT_CTOR(TaskCreateSupportTicket);
}

TaskCreateSupportTicket::~TaskCreateSupportTicket() {
  MVPN_COUNT_DTOR(TaskCreateSupportTicket);
}

void TaskCreateSupportTicket::run() {
  logger.debug() << "Sending the support ticket";

  NetworkRequest* request = NetworkRequest::createForSupportTicket(
      this, m_email, m_subject, m_issueText, m_logs, m_category);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to create support ticket" << error;
            MozillaVPN::instance()->createTicketAnswerRecieved(false);
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray&) {
            logger.debug() << "Support ticket created";
            MozillaVPN::instance()->createTicketAnswerRecieved(true);
            emit completed();
          });
}
