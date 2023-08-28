/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktasktransfer.h"

#include <QByteArray>
#include <QDnsLookup>
#include <QHostAddress>
#include <QScopeGuard>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "uploaddatagenerator.h"

namespace {
Logger logger("BenchmarkTaskTransfer");
}

BenchmarkTaskTransfer::BenchmarkTaskTransfer(const QString& name,
                                             BenchmarkType type,
                                             const QUrl& url)
    : BenchmarkTask(name, Constants::BENCHMARK_MAX_DURATION_TRANSFER),
      m_type(type),
      m_dnsLookup(QDnsLookup::A, url.host()),
      m_url(url) {
  MZ_COUNT_CTOR(BenchmarkTaskTransfer);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskTransfer::handleState);
  connect(&m_dnsLookup, &QDnsLookup::finished, this,
          &BenchmarkTaskTransfer::dnsLookupFinished);
}

BenchmarkTaskTransfer::~BenchmarkTaskTransfer() {
  MZ_COUNT_DTOR(BenchmarkTaskTransfer);
}

void BenchmarkTaskTransfer::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
#if defined(MZ_DUMMY) || defined(MZ_ANDROID) || defined(MZ_WASM)
// #  if QT_VERSION >= 0x060500
// #    error Check if QT added support for QDnsLookup::lookup() on Android
// #  endif

    createNetworkRequest();
#else
    // Start DNS resolution
    m_dnsLookup.lookup();
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
      request = new NetworkRequest(this);
      request->get(m_url);
      break;
    }
    case BenchmarkUpload: {
      UploadDataGenerator* uploadData =
          new UploadDataGenerator(Constants::BENCHMARK_MAX_BYTES_UPLOAD);

      if (!uploadData->open(UploadDataGenerator::ReadOnly)) {
        emit finished(0, true);
        emit completed();
      };
      request = new NetworkRequest(this, 200);
      request->requestInternal().setHeader(QNetworkRequest::ContentTypeHeader,
                                           "application/x-www-form-urlencoded");
      request->post(m_url, uploadData);
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
      QUrl requestUrl(m_url);
      QString hostname = requestUrl.host();

      // Rewrite the request URL to use an explicit host address.
      if (record.value().protocol() == QAbstractSocket::IPv6Protocol) {
        requestUrl.setHost("[" + record.value().toString() + "]");
      } else {
        requestUrl.setHost(record.value().toString());
      }

      request = new NetworkRequest(this, 200);
      request->requestInternal().setRawHeader("Host", hostname.toLocal8Bit());
      request->requestInternal().setPeerVerifyName(hostname);

      request->get(requestUrl);
      break;
    }
    case BenchmarkUpload: {
      UploadDataGenerator* uploadData =
          new UploadDataGenerator(Constants::BENCHMARK_MAX_BYTES_UPLOAD);

      if (!uploadData->open(UploadDataGenerator::ReadOnly)) {
        emit finished(0, true);
        emit completed();
      };
      QUrl requestUrl(m_url);
      QString hostname = requestUrl.host();

      // Rewrite the request URL to use an explicit host address.
      if (record.value().protocol() == QAbstractSocket::IPv6Protocol) {
        requestUrl.setHost("[" + record.value().toString() + "]");
      } else {
        requestUrl.setHost(record.value().toString());
      }

      request = new NetworkRequest(this, 200);
      request->requestInternal().setHeader(QNetworkRequest::ContentTypeHeader,
                                           "application/x-www-form-urlencoded");

      request->requestInternal().setRawHeader("Host", hostname.toLocal8Bit());
      request->requestInternal().setPeerVerifyName(hostname);

      request->post(requestUrl, uploadData);
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

  if (m_dnsLookup.error() == QDnsLookup::OperationCancelledError) {
    guard.dismiss();
    return;
  }

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
#ifdef MZ_DEBUG
  logger.debug() << "Transfer progressed:" << bytesSent << "(transferred)"
                 << bytesTotal << "(total)";
#else
  Q_UNUSED(bytesTotal);
#endif

  NetworkRequest* request = qobject_cast<NetworkRequest*>(sender());
  if (request != nullptr) {
    request->discardData();
  }

  if (bytesSent > 0) {
    m_bytesTransferred = bytesSent;
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
#ifndef MZ_WASM
                            || bitsPerSec == 0
#endif
      ;

  logger.debug() << "Transfer completed" << bitsPerSec << "baud";

  if (m_requests.isEmpty()) {
    emit finished(bitsPerSec, hasUnexpectedError);
    emit completed();
  }
}
