/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskmasqueservers.h"

#include <QHostAddress>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QSet>
#include <QUrl>

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskMasqueServers");
}

TaskMasqueServers::TaskMasqueServers(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskMasqueServers"),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskMasqueServers);
}

TaskMasqueServers::~TaskMasqueServers() {
  MZ_COUNT_DTOR(TaskMasqueServers);
  for (int id : m_lookupIds) {
    QHostInfo::abortHostLookup(id);
  }
}

void TaskMasqueServers::run() {
  // Remote Settings is a public, unauthenticated endpoint, so no auth() here.
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->get(QUrl(Constants::MASQUE_SERVERS_URL));

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to retrieve MASQUE servers";
            REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "MASQUE servers obtained";
            resolveHostnames(data);
          });
}

void TaskMasqueServers::resolveHostnames(const QByteArray& changeset) {
  QJsonDocument doc = QJsonDocument::fromJson(changeset);
  if (!doc.isObject()) {
    logger.warning() << "Invalid MASQUE changeset";
    emit completed();
    return;
  }
  m_root = doc.object();

  // Collect every unique hostname advertised in the changeset.
  QSet<QString> hostnames;
  for (const QJsonValue& countryValue : m_root.value("changes").toArray()) {
    for (const QJsonValue& cityValue :
         countryValue.toObject().value("cities").toArray()) {
      for (const QJsonValue& serverValue :
           cityValue.toObject().value("servers").toArray()) {
        QString hostname = serverValue.toObject().value("hostname").toString();
        if (!hostname.isEmpty()) {
          hostnames.insert(hostname);
        }
      }
    }
  }

  if (hostnames.isEmpty()) {
    logger.debug() << "No MASQUE hostnames to resolve";
    MozillaVPN::instance()->masqueServersFetched(changeset);
    emit completed();
    return;
  }

#ifdef MZ_WASM
  // QHostInfo uses dlopen() to run the DNS lookup, which does not work on WASM.
  // Store the servers without resolved addresses.
  MozillaVPN::instance()->masqueServersFetched(changeset);
  emit completed();
  return;
#endif

  m_pendingLookups = hostnames.count();
  for (const QString& hostname : hostnames) {
    int id = QHostInfo::lookupHost(hostname, this,
                                   SLOT(dnsLookupCompleted(QHostInfo)));
    m_lookupIds.append(id);
  }
}

void TaskMasqueServers::dnsLookupCompleted(const QHostInfo& hostInfo) {
  if (hostInfo.error() != QHostInfo::NoError) {
    logger.warning() << "DNS lookup failed for" << hostInfo.hostName() << ":"
                     << hostInfo.errorString();
  } else {
    // Keep the first address of each family for this hostname.
    for (const QHostAddress& address : hostInfo.addresses()) {
      if (address.isNull()) {
        continue;
      }
      if (address.protocol() == QAbstractSocket::IPv4Protocol &&
          !m_ipv4.contains(hostInfo.hostName())) {
        m_ipv4.insert(hostInfo.hostName(), address.toString());
      } else if (address.protocol() == QAbstractSocket::IPv6Protocol &&
                 !m_ipv6.contains(hostInfo.hostName())) {
        m_ipv6.insert(hostInfo.hostName(), address.toString());
      }
    }
  }

  Q_ASSERT(m_pendingLookups > 0);
  if (--m_pendingLookups == 0) {
    m_lookupIds.clear();
    finalize();
  }
}

void TaskMasqueServers::finalize() {
  // Inject the resolved addresses back into each server entry.
  QJsonArray changes = m_root.value("changes").toArray();
  for (int i = 0; i < changes.size(); ++i) {
    QJsonObject country = changes.at(i).toObject();
    QJsonArray cities = country.value("cities").toArray();
    for (int j = 0; j < cities.size(); ++j) {
      QJsonObject city = cities.at(j).toObject();
      QJsonArray servers = city.value("servers").toArray();
      for (int k = 0; k < servers.size(); ++k) {
        QJsonObject server = servers.at(k).toObject();
        QString hostname = server.value("hostname").toString();
        if (m_ipv4.contains(hostname)) {
          server["ipv4_addr_in"] = m_ipv4.value(hostname);
        }
        if (m_ipv6.contains(hostname)) {
          server["ipv6_addr_in"] = m_ipv6.value(hostname);
        }
        servers[k] = server;
      }
      city["servers"] = servers;
      cities[j] = city;
    }
    country["cities"] = cities;
    changes[i] = country;
  }
  m_root["changes"] = changes;

  QByteArray data = QJsonDocument(m_root).toJson(QJsonDocument::Compact);
  MozillaVPN::instance()->masqueServersFetched(data);
  emit completed();
}
