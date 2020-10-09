/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QtDebug>

// Any 6 hours, a new check
constexpr uint32_t RELEASE_MONITOR_SEC = 21600;

void ReleaseMonitor::runSoon()
{
    qDebug() << "ReleaseManager - Scheduling a quick timer";
    QTimer::singleShot(0, [this] { runInternal(); });
}

void ReleaseMonitor::runInternal()
{
    qDebug() << "ReleaseMonitor started";

    NetworkRequest *request = NetworkRequest::createForVersions(MozillaVPN::instance());

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Versions request failed" << error;
        schedule();
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        qDebug() << "Account request completed";
        processData(data);
        schedule();
    });
}

void ReleaseMonitor::schedule()
{
    qDebug() << "ReleaseMonitor scheduling";
    QTimer::singleShot(RELEASE_MONITOR_SEC * 1000, [this] { runInternal(); });
}

void ReleaseMonitor::processData(const QByteArray &data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    Q_ASSERT(json.isObject());
    QJsonObject obj = json.object();

    QString platformKey =
#ifdef IOS_INTEGRATION
        "ios"
#elif __APPLE__
        "macos"
#elif __linux__
        "linux"
#else
        "dummy"
#endif
        ;

    if (!obj.contains(platformKey)) {
        qDebug() << "No key" << platformKey;
        return;
    }

    QJsonValue platformDataValue = obj.take(platformKey);
    Q_ASSERT(platformDataValue.isObject());
    QJsonObject platformData = platformDataValue.toObject();

    Q_ASSERT(platformData.contains("latest"));
    QJsonValue latestValue = platformData.take("latest");
    Q_ASSERT(latestValue.isObject());
    QJsonObject latestData = latestValue.toObject();

    Q_ASSERT(latestData.contains("version"));
    QJsonValue latestVersionValue = latestData.take("version");
    Q_ASSERT(latestVersionValue.isString());

    Q_ASSERT(platformData.contains("minimum"));
    QJsonValue minimumValue = platformData.take("minimum");
    Q_ASSERT(minimumValue.isObject());
    QJsonObject minimumData = minimumValue.toObject();

    Q_ASSERT(minimumData.contains("version"));
    QJsonValue minimumVersionValue = minimumData.take("version");
    Q_ASSERT(minimumVersionValue.isString());

    double latestVersion = latestVersionValue.toString().toDouble();
    double minimumVersion = minimumVersionValue.toString().toDouble();
    double currentVersion = QString(APP_VERSION).toDouble();

    qDebug() << "Latest version:" << latestVersion;
    qDebug() << "Minimum version:" << minimumVersion;
    qDebug() << "Current version:" << currentVersion;

    if (currentVersion < minimumVersion) {
        qDebug() << "ReleaseMonitor - update required";
        MozillaVPN::instance()->setUpdateRecommended(false);
        MozillaVPN::instance()->controller()->updateRequired();
        return;
    }

    qDebug() << "Update recommended: " << (currentVersion < latestVersion);
    MozillaVPN::instance()->setUpdateRecommended(currentVersion < latestVersion);
}
