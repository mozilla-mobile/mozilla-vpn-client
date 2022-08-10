/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasklatency.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "pingsenderfactory.h"

#include <QDateTime>

constexpr const uint32_t SERVER_LATENCY_TIMEOUT_MSEC = 5000;

namespace {
Logger logger(LOG_MAIN, "TaskLatency");
}

TaskLatency::TaskLatency() : Task("TaskLatency") {
  MVPN_COUNT_CTOR(TaskLatency);
}

TaskLatency::~TaskLatency() { MVPN_COUNT_DTOR(TaskLatency); }

void TaskLatency::run() {
  m_pingSender = PingSenderFactory::create(QHostAddress(), this);
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();

  connect(m_pingSender, SIGNAL(recvPing(quint16)), this,
          SLOT(recvPing(quint16)));
  connect(m_pingSender, SIGNAL(criticalPingError()), this,
          SLOT(criticalPingError()));
  connect(&m_timeout, &QTimer::timeout, this, [&] {
    m_pingReplyList.clear();
    emit completed();
  });

  // Send a ping to every server.
  quint16 seq = 1;
  for (const Server& server : scm->servers()) {
    m_pingSender->sendPing(QHostAddress(server.ipv4AddrIn()), seq);

    ServerPingRecord record;
    record.publicKey = server.publicKey();
    record.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_pingReplyList[seq] = record;
    seq++;
  }

    m_timeout.start(SERVER_LATENCY_TIMEOUT_MSEC);
}

void TaskLatency::recvPing(quint16 sequence) {
  ServerPingRecord record = m_pingReplyList.take(sequence);
  if (record.publicKey.isEmpty()) {
    // We didn't expect this ping reply.
    return;
  }

  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  quint64 latency = QDateTime::currentMSecsSinceEpoch() - record.timestamp;
  scm->setServerLatency(record.publicKey, latency);

  if (m_pingReplyList.isEmpty()) {
    emit completed();
  }
}

void TaskLatency::criticalPingError() {
  logger.info() << "Encountered Unrecoverable ping error";
}
