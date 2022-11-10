/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskipfinder.h"
#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"

#include <QHostAddress>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>

namespace {
Logger logger(LOG_NETWORKING, "TaskIPFinder");
}

TaskIPFinder::TaskIPFinder() : Task("TaskIPFinder") {
  MVPN_COUNT_CTOR(TaskIPFinder);

  // Queued to avoid this task to be deleted before the processing of the slots.
  connect(this, &TaskIPFinder::operationCompleted, this, &Task::completed,
          Qt::QueuedConnection);
}

TaskIPFinder::~TaskIPFinder() {
  MVPN_COUNT_DTOR(TaskIPFinder);
  if (m_lookupId > -1) {
    QHostInfo::abortHostLookup(m_lookupId);
  }
}

void TaskIPFinder::run() {
  logger.debug() << "Starting the ip-lookup";

#ifdef MVPN_WASM
  // QHostInfo uses dlopen() to run the DNS lookup. This does not work on WASM.
  // Let's fake the result.
  emit operationCompleted("1.2.3.4", "a1f:ea75:ca75", "Mordor");
  return;
#endif

  QUrl url(NetworkRequest::apiBaseUrl());
  m_lookupId = QHostInfo::lookupHost(url.host(), this,
                                     SLOT(dnsLookupCompleted(QHostInfo)));
}

void TaskIPFinder::dnsLookupCompleted(const QHostInfo& hostInfo) {
  logger.debug() << "DNS lookup completed";

  m_lookupId = -1;

  if (hostInfo.error() != QHostInfo::NoError) {
    logger.error() << "Unable to perform a DNS Lookup:"
                   << hostInfo.errorString();
    emit operationCompleted(QString(), QString(), QString());
    return;
  }

  for (const QHostAddress& address : hostInfo.addresses()) {
    if (address.isNull() || address.isBroadcast()) continue;

    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
      logger.debug() << "Ipv4:" << logger.sensitive(address.toString());
      createRequest(address, false);
    }

    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
      logger.debug() << "Ipv6:" << logger.sensitive(address.toString());
      createRequest(address, true);
    }
  }

  if (m_requestCount == 0) {
    logger.debug() << "No requests created. Let's abort the lookup";
    emit operationCompleted(QString(), QString(), QString());
    return;
  }
}

void TaskIPFinder::createRequest(const QHostAddress& address, bool ipv6) {
  NetworkRequest* request = NetworkRequest::createForIpInfo(this, address);

  ++m_requestCount;

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "IP address request failed" << error;

            // This lookup has been canceled in the meantime.
            if (m_requestCount == 0) {
              return;
            }

            ErrorHandler::ErrorType errorType =
                ErrorHandler::toErrorType(error);
            if (errorType == ErrorHandler::AuthenticationError) {
              ErrorHandler::instance()->errorHandle(errorType);
            }

            m_requestCount = 0;
            emit operationCompleted(QString(), QString(), QString());
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this, ipv6](const QByteArray& data) {
            logger.debug() << "IP address request completed";

            // This lookup has been canceled in the meantime.
            if (m_requestCount == 0) {
              return;
            }

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isObject()) {
              QJsonObject obj = json.object();

              QJsonValue country = obj.take("country");
              QJsonValue value = obj.value("ip");

              m_results.append(IPLookup{country.toString().toLower(),
                                        value.toString(), ipv6});
            }

            --m_requestCount;
            if (m_requestCount == 0) {
              completeLookup();
            }
          });
}

void TaskIPFinder::completeLookup() {
  Q_ASSERT(m_requestCount == 0);

  logger.debug() << "Lookup completed!";

  QString ipv4Address;
  QString ipv6Address;
  QString country;

  // In theory, we should have one ipv4 result and maybe an ipv6 too, but we do
  // not know how the API host has been resolved. Let's use only the first
  // result per type.
  for (const IPLookup& ipLookup : m_results) {
    if (ipLookup.m_ipAddress.isEmpty()) {
      continue;
    }

    if (ipLookup.m_ipv6 && ipv6Address.isEmpty()) {
      ipv6Address = ipLookup.m_ipAddress;
    }

    if (!ipLookup.m_ipv6 && ipv4Address.isEmpty()) {
      ipv4Address = ipLookup.m_ipAddress;
    }

    if (country.isEmpty()) {
      country = ipLookup.m_country;
    }
  }

  emit operationCompleted(ipv4Address, ipv6Address, country);
}
