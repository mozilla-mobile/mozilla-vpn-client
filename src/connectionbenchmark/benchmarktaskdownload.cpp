/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskdownload.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#include <QByteArray>
#include <QDnsLookup>
#include <QHostAddress>

constexpr const char* MULLVAD_DEFAULT_DNS = "10.64.0.1";

namespace {
Logger logger(LOG_MAIN, "BenchmarkTaskDownload");
}

BenchmarkTaskDownload::BenchmarkTaskDownload(const QUrl& url)
    : BenchmarkTask(Constants::BENCHMARK_MAX_DURATION_DOWNLOAD),
      m_dnsLookup(QDnsLookup::A, url.host()),
      m_fileUrl(url) {
  MVPN_COUNT_CTOR(BenchmarkTaskDownload);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskDownload::handleState);
  connect(&m_dnsLookup, &QDnsLookup::finished, this,
          &BenchmarkTaskDownload::dnsLookupFinished);
}

BenchmarkTaskDownload::~BenchmarkTaskDownload() {
  MVPN_COUNT_DTOR(BenchmarkTaskDownload);
}

void BenchmarkTaskDownload::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
    // Start DNS resolution
    m_dnsLookup.setNameserver(QHostAddress(MULLVAD_DEFAULT_DNS));
    m_dnsLookup.lookup();
  } else if (state == BenchmarkTask::StateInactive) {
    for (NetworkRequest* request : m_requests) {
      request->abort();
    }
    m_requests.clear();
  }
}

void BenchmarkTaskDownload::dnsLookupFinished() {
  if (m_dnsLookup.error() != QDnsLookup::NoError) {
    logger.error() << "DNS Lookup Failed:" << m_dnsLookup.errorString();
    emit finished(0, true);
    emit completed();
    return;
  }
  if (m_dnsLookup.hostAddressRecords().isEmpty()) {
    logger.error() << "DNS Lookup Failed: no records";
    emit finished(0, true);
    emit completed();
    return;
  }
  logger.debug() << "DNS Lookup Finished";

  for (const QDnsHostAddressRecord& record : m_dnsLookup.hostAddressRecords()) {
    logger.debug() << "Host record:" << record.value().toString();

    NetworkRequest* request = NetworkRequest::createForGetHostAddress(
        this, m_fileUrl.toString(), record.value());

    connect(request, &NetworkRequest::requestUpdated, this,
            &BenchmarkTaskDownload::downloadProgressed);
    connect(request, &NetworkRequest::requestFailed, this,
            &BenchmarkTaskDownload::downloadReady);
    connect(request, &NetworkRequest::requestCompleted, this,
            [&](const QByteArray& data) {
              downloadReady(QNetworkReply::NoError, data);
            });

    logger.debug() << "Starting request:" << (void*)request;
    m_requests.append(request);
  }

  m_elapsedTimer.start();
}

void BenchmarkTaskDownload::downloadProgressed(qint64 bytesReceived,
                                               qint64 bytesTotal,
                                               QNetworkReply* reply) {
#ifdef MVPN_DEBUG
  logger.debug() << "Handle progressed:" << bytesReceived << "(received)"
                 << bytesTotal << "(total)";
#endif

  // Count and discard downloaded data
  m_bytesReceived += reply->skip(bytesTotal);
}

void BenchmarkTaskDownload::downloadReady(QNetworkReply::NetworkError error,
                                          const QByteArray& data) {
  logger.debug() << "Download ready" << error;
  Q_UNUSED(data);

  NetworkRequest* request = qobject_cast<NetworkRequest*>(QObject::sender());
  m_requests.removeAll(request);

  quint64 bitsPerSec = 0;
  double msecs = static_cast<double>(m_elapsedTimer.elapsed());
  if (m_bytesReceived > 0 && msecs > 0) {
    bitsPerSec = static_cast<quint64>(static_cast<double>(m_bytesReceived * 8) /
                                      (msecs / 1000.00));
  }

  bool hasUnexpectedError = (error != QNetworkReply::NoError &&
                             error != QNetworkReply::OperationCanceledError &&
                             error != QNetworkReply::TimeoutError) ||
                            bitsPerSec == 0;
  logger.debug() << "Download completed" << bitsPerSec << "baud";

  if (m_requests.isEmpty()) {
    emit finished(bitsPerSec, hasUnexpectedError);
    emit completed();
  }
}
