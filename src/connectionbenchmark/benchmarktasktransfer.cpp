/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktasktransfer.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "uploaddatagenerator.h"

#include <QByteArray>
#include <QDnsLookup>
#include <QHostAddress>
#include <QScopeGuard>

#if !defined(MVPN_DUMMY) && !defined(MVPN_ANDROID) && !defined(MVPN_WASM)
constexpr const char* MULLVAD_DEFAULT_DNS = "10.64.0.1";
#endif

namespace {
Logger logger(LOG_MAIN, "BenchmarkTaskTransfer");
}

BenchmarkTaskTransfer::BenchmarkTaskTransfer(const QString& name,
                                             BenchmarkType type,
                                             const QUrl& url)
    : BenchmarkTask(name, Constants::BENCHMARK_MAX_DURATION_TRANSFER),
      m_type(type),
      m_dnsLookup(QDnsLookup::A, url.host()),
      m_url(url) {
  MVPN_COUNT_CTOR(BenchmarkTaskTransfer);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskTransfer::handleState);
  connect(&m_dnsLookup, &QDnsLookup::finished, this,
          &BenchmarkTaskTransfer::dnsLookupFinished);
}

BenchmarkTaskTransfer::~BenchmarkTaskTransfer() {
  MVPN_COUNT_DTOR(BenchmarkTaskTransfer);
}

void BenchmarkTaskTransfer::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
#if defined(MVPN_DUMMY) || defined(MVPN_ANDROID) || defined(MVPN_WASM)
    createNetworkRequest();
#else
    // Start DNS resolution
    m_dnsLookup.setNameserver(QHostAddress(MULLVAD_DEFAULT_DNS));
    m_dnsLookup.lookup();
#endif

#if QT_VERSION >= 0x060500
#  error Check if QT added support for QDnsLookup::lookup() on Android
#endif

    m_elapsedTimer.start();
  } else if (state == BenchmarkTask::StateInactive) {
    for (NetworkRequest* request : m_requests) {
      request->abort();
    }
    m_requests.clear();
    m_dnsLookup.abort();
  }
}

void BenchmarkTaskTransfer::createNetworkRequest() {
  logger.debug() << "Create network request";

  NetworkRequest* request = nullptr;
  switch (m_type) {
    case BenchmarkDownload: {
      request = NetworkRequest::createForGetUrl(this, m_url.toString());
      break;
    }
    case BenchmarkUpload: {
      UploadDataGenerator* uploadData =
          new UploadDataGenerator(Constants::BENCHMARK_MAX_BITS_UPLOAD);

      if (!uploadData->open(UploadDataGenerator::ReadOnly)) {
        emit finished(0, true);
        emit completed();
      };
      request = NetworkRequest::createForUploadData(this, m_url.toString(),
                                                    uploadData);
      break;
    }
  }

  Q_ASSERT(request);
  connectNetworkRequest(request);
}

void BenchmarkTaskTransfer::createNetworkRequestWithRecord(
    const QDnsHostAddressRecord& record) {
  logger.debug() << "Create network request with record";

  NetworkRequest* request = nullptr;
  switch (m_type) {
    case BenchmarkDownload: {
      request = NetworkRequest::createForGetHostAddress(this, m_url.toString(),
                                                        record.value());
      break;
    }
    case BenchmarkUpload: {
      UploadDataGenerator* uploadData =
          new UploadDataGenerator(Constants::BENCHMARK_MAX_BITS_UPLOAD);

      if (!uploadData->open(UploadDataGenerator::ReadOnly)) {
        emit finished(0, true);
        emit completed();
      };
      request = NetworkRequest::createForUploadDataHostAddress(
          this, m_url.toString(), uploadData, record.value());
      break;
    }
    default: {
      logger.error() << "Unhandled benchmark type";
      break;
    }
  }

  Q_ASSERT(request);
  connectNetworkRequest(request);
}

void BenchmarkTaskTransfer::connectNetworkRequest(NetworkRequest* request) {
  logger.debug() << "Connect network requests";

  switch (m_type) {
    case BenchmarkDownload: {
      connect(request, &NetworkRequest::requestUpdated, this,
              &BenchmarkTaskTransfer::transferProgressed);
      break;
    }
    case BenchmarkUpload: {
      connect(request, &NetworkRequest::uploadProgressed, this,
              &BenchmarkTaskTransfer::transferProgressed);
      break;
    }
    default: {
      logger.error() << "Unhandled benchmark type";
      break;
    }
  }
  connect(request, &NetworkRequest::requestFailed, this,
          &BenchmarkTaskTransfer::transferReady);
  connect(request, &NetworkRequest::requestCompleted, this,
          [&](const QByteArray& data) {
            transferReady(QNetworkReply::NoError, data);
          });

  logger.debug() << "Starting request";
  m_requests.append(request);
}

void BenchmarkTaskTransfer::dnsLookupFinished() {
  auto guard = qScopeGuard([&] {
    emit finished(0, true);
    emit completed();
  });

  if (m_dnsLookup.error() != QDnsLookup::NoError) {
    logger.error() << "DNS Lookup Failed:" << m_dnsLookup.errorString();
    return;
  }
  if (m_dnsLookup.hostAddressRecords().isEmpty()) {
    logger.error() << "DNS Lookup Failed: no records";
    return;
  }
  if (state() != BenchmarkTask::StateActive) {
    logger.warning() << "DNS Lookup finished after task aborted";
    return;
  }

  logger.debug() << "DNS Lookup Finished";
  for (const QDnsHostAddressRecord& record : m_dnsLookup.hostAddressRecords()) {
    logger.debug() << "Host record:" << record.value().toString();
    createNetworkRequestWithRecord(record);
  }

  m_elapsedTimer.start();
  guard.dismiss();
}

void BenchmarkTaskTransfer::transferProgressed(qint64 bytesSent,
                                               qint64 bytesTotal,
                                               QNetworkReply* reply) {
#ifdef MVPN_DEBUG
  logger.debug() << "Transfer progressed:" << bytesSent << "(transferred)"
                 << bytesTotal << "(total)";
#else
  Q_UNUSED(bytesTotal);
#endif

  switch (m_type) {
    case BenchmarkDownload: {
      // Count and discard downloaded data
      m_bytesTransferred += reply->skip(bytesTotal);
      break;
    }
    case BenchmarkUpload: {
      Q_UNUSED(reply);
      m_bytesTransferred += bytesSent;
      break;
    }
    default: {
      logger.error() << "Unhandled benchmark type";
      break;
    }
  }
}

void BenchmarkTaskTransfer::transferReady(QNetworkReply::NetworkError error,
                                          const QByteArray& data) {
  logger.debug() << "Transfer ready" << error;
  Q_UNUSED(data);

  NetworkRequest* request = qobject_cast<NetworkRequest*>(QObject::sender());
  m_requests.removeOne(request);

  quint64 bitsPerSec = 0;
  double msecs = static_cast<double>(m_elapsedTimer.elapsed());
  if (m_bytesTransferred > 0 && msecs > 0) {
    bitsPerSec = static_cast<quint64>(
        static_cast<double>(m_bytesTransferred * 8) / (msecs / 1000.00));
  }

  bool hasUnexpectedError = (error != QNetworkReply::NoError &&
                             error != QNetworkReply::OperationCanceledError &&
                             error != QNetworkReply::TimeoutError)
#ifndef MVPN_WASM
                            || bitsPerSec == 0
#endif
      ;

  logger.debug() << "Transfer completed" << bitsPerSec << "baud";

  if (m_requests.isEmpty()) {
    emit finished(bitsPerSec, hasUnexpectedError);
    emit completed();
  }
}
