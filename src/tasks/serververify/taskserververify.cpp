/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskserververify.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "pingsenderfactory.h"

constexpr const int SERVER_VERIFY_MAX_ATTEMPTS = 4;
constexpr const int SERVER_VERIFY_INTERVAL_MSEC = 1000;

namespace {
Logger logger(LOG_MAIN, "TaskServerVerify");
}

TaskServerVerify::TaskServerVerify(const QString& serverPublicKey)
    : Task("TaskServerVerify") {
  MVPN_COUNT_CTOR(TaskServerVerify);

  logger.error() << "Attempting to verify" << logger.keys(serverPublicKey);
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  m_server = scm->server(serverPublicKey);
}

TaskServerVerify::~TaskServerVerify() { MVPN_COUNT_DTOR(TaskServerVerify); }

void TaskServerVerify::run() {
  if (!m_server.initialized()) {
    logger.error() << "Attempting to verify an invalid server";
    emit completed();
  }

  m_pingReceived = 0;
  m_pingAttempts = SERVER_VERIFY_MAX_ATTEMPTS;
  m_pingSender = PingSenderFactory::create(QHostAddress(), this);
  if (!m_pingSender->isValid()) {
    logger.error() << "Failed to create PingSender for server verification";
    emit completed();
  }

  connect(&m_pingTimer, &QTimer::timeout, this, [&]() {
    if (m_pingAttempts) {
      m_pingAttempts--;
      m_pingSender->sendPing(QHostAddress(m_server.ipv4AddrIn()),
                             1000 + m_pingAttempts);
    } else {
      logger.info() << "Received" << m_pingReceived << "out of"
                    << SERVER_VERIFY_MAX_ATTEMPTS << "ping attempts";
      emit completed();
    }
  });

  connect(m_pingSender, &PingSender::recvPing, this, [&](quint16 sequence) {
    Q_UNUSED(sequence);
    m_pingReceived++;
    logger.info() << "Received ping from"
                  << logger.sensitive(m_server.hostname());
  });

  logger.info() << "Starting ping to" << logger.sensitive(m_server.hostname());
  m_pingTimer.start(SERVER_VERIFY_INTERVAL_MSEC);
}
