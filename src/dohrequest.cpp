/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dohrequest.h"
#include "logger.h"
#include "networkrequest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>

constexpr const char *DOH_HOST = "mozilla.cloudflare-dns.com";
constexpr const char *DOH_URL = "https://1.1.1.1/dns-query";

constexpr uint32_t DOH_TYPE_A = 1;
constexpr uint32_t DOH_TYPE_AAAA = 28;

namespace {
Logger logger(LOG_NETWORKING, "DOHRequest");
}

DOHRequest::DOHRequest(QObject *parent) : QObject(parent)
{
}

void DOHRequest::resolve(const QString &host)
{
    logger.log() << "Resolving" << host;
    resolveInternal(host, DOH_TYPE_A);
    resolveInternal(host, DOH_TYPE_AAAA);
}

void DOHRequest::resolveInternal(const QString &host, uint32_t type)
{
    Q_ASSERT(type == DOH_TYPE_A || type == DOH_TYPE_AAAA);

    QUrlQuery query;
    query.addQueryItem("name", host);
    query.addQueryItem("type", QString::number(type));

    QUrl url(DOH_URL);
    url.setQuery(query);

    NetworkRequest *request = NetworkRequest::createForDOH(this, url, DOH_HOST);
    connect(request,
            &NetworkRequest::requestFailed,
            [this, type](QNetworkReply::NetworkError error) {
                logger.log() << "DOH request failed" << error;
                maybeCompleted(type);
            });

    connect(request, &NetworkRequest::requestCompleted, [this, type](const QByteArray &data) {
        logger.log() << "DOH request completed";
        parseResult(data);
        maybeCompleted(type);
    });
}

void DOHRequest::maybeCompleted(uint32_t type)
{
    if (type == DOH_TYPE_A) {
        m_ipv4AddressesCompleted = true;
    } else {
        Q_ASSERT(type == DOH_TYPE_AAAA);
        m_ipv6AddressesCompleted = true;
    }

    if (m_ipv4AddressesCompleted && m_ipv6AddressesCompleted) {
        emit completed(m_ipv4Addresses, m_ipv6Addresses);
        deleteLater();
    }
}

void DOHRequest::parseResult(const QByteArray &data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    if (!json.isObject()) {
        logger.log() << "The response is not a valid json";
        return;
    }

    QJsonObject obj = json.object();

    QJsonValue status = obj.take("Status");
    if (!status.isDouble()) {
        logger.log() << "Invalid status";
        return;
    }

    if (status.toInt() != 0) {
        logger.log() << "Lookup error:" << status.toInt();
        return;
    }

    QJsonValue answers = obj.take("Answer");
    if (!answers.isArray()) {
        logger.log() << "Invalid answer";
        return;
    }

    for (QJsonValue answer : answers.toArray()) {
        if (!answer.isObject()) {
            logger.log() << "Invalid single answer";
            return;
        }

        QJsonObject answerObj = answer.toObject();

        QJsonValue type = answerObj.take("type");
        if (!type.isDouble()) {
            logger.log() << "Invalid answer type";
            return;
        }

        if (type.toInt() != DOH_TYPE_A && type.toInt() != DOH_TYPE_AAAA) {
            continue;
        }

        QJsonValue data = answerObj.take("data");
        if (!data.isString()) {
            logger.log() << "Invalid answer data";
            return;
        }

        if (type.toInt() == DOH_TYPE_A) {
            m_ipv4Addresses.append(data.toString());
        } else {
            Q_ASSERT(type.toInt() == DOH_TYPE_AAAA);
            m_ipv6Addresses.append(data.toString());
        }
    }
}
