/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pingsender.h"
#include "logger.h"

// QProcess is not supported on iOS
#ifndef MVPN_IOS
#  include <QProcess>
#endif

namespace {
Logger logger({LOG_LINUX, LOG_NETWORKING}, "PingSender");
}

quint16 PingSender::inetChecksum(const void* data, size_t len) {
  int nleft, sum;
  quint16* w;
  union {
    quint16 us;
    quint8 uc[2];
  } last;
  quint16 answer;

  nleft = static_cast<int>(len);
  sum = 0;
  w = (quint16*)data;

  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (nleft == 1) {
    last.uc[0] = *(quint8*)w;
    last.uc[1] = 0;
    sum += last.us;
  }

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
  sum += (sum >> 16);                 /* add carry */
  answer = ~sum;                      /* truncate to 16 bits */
  return (answer);
}

// QProcess is not supported on iOS
#ifndef MVPN_IOS
// Send a ping by launching the "ping" command.
void PingSender::genericSendPing(const QStringList& args, qint16 sequence) {
  QProcess* process = new QProcess(this);
  process->setProcessChannelMode(QProcess::MergedChannels);
  process->start("ping", args, QIODevice::ReadOnly);

  connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          this, [this, sequence](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) emit recvPing(sequence);
          });
  connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          process, &QObject::deleteLater);
  connect(
      process, &QProcess::errorOccurred, this,
      [this, sequence](QProcess::ProcessError error) {
        switch (error) {
          case QProcess::ProcessError::Crashed:
            logger.error() << "Failed to use native Ping: Crashed";
            break;
          case QProcess::ProcessError::FailedToStart:
            logger.error() << "Failed to use native Ping: FailedToStart";
            break;
          case QProcess::ProcessError::ReadError:
            logger.error() << "Failed to use native Ping: ReadError";
            break;
          case QProcess::ProcessError::Timedout:
            logger.error() << "Failed to use native Ping: Timedout";
            break;
          case QProcess::ProcessError::UnknownError:
            logger.error() << "Failed to use native Ping: UnknownError";
            break;
          case QProcess::ProcessError::WriteError:
            logger.error() << "Failed to use native Ping: WriteError";
            break;
        };
        // Using generic ping is a fallback anyway, if we fail here
        // we won't be able to infer any info about connection health -
        // so lets just return a fake ping, so the user can use the app
        // but without the "no connection" / "no confirm"
        logger.info() << "Pushing fake ping response";
        emit recvPing(sequence);
      },
      Qt::ConnectionType::QueuedConnection);
  connect(process, &QProcess::errorOccurred, process, &QObject::deleteLater);

  // Ensure any lingering pings are cleaned up when the PingSender is destroyed
  connect(this, &QObject::destroyed, process, [process] {
    process->terminate();
    if (!process->waitForFinished(100)) {
      process->kill();
    }
  });
}
#endif
