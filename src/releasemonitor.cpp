/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"
#include "constants.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

namespace {
Logger logger(LOG_MAIN, "ReleaseMonitor");
}

void ReleaseMonitor::runSoon()
{
    logger.log() << "ReleaseManager - Scheduling a quick timer";
    QTimer::singleShot(0, [this] { runInternal(); });
}

void ReleaseMonitor::runInternal()
{
    logger.log() << "ReleaseMonitor started";

    NetworkRequest *request = NetworkRequest::createForVersions(MozillaVPN::instance());

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        logger.log() << "Versions request failed" << error;
        schedule();
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        logger.log() << "Account request completed";

        if (!processData(data)) {
            logger.log() << "Ignore failure.";
        }

        schedule();
    });
}

void ReleaseMonitor::schedule()
{
    logger.log() << "ReleaseMonitor scheduling";
    QTimer::singleShot(Constants::RELEASE_MONITOR_MSEC, [this] { runInternal(); });
}

bool ReleaseMonitor::processData(const QByteArray &data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    if (!json.isObject()) {
        logger.log() << "A valid JSON object expected";
        return false;
    }

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
        logger.log() << "No key" << platformKey;
        return false;
    }

    QJsonValue platformDataValue = obj.take(platformKey);
    if (!platformDataValue.isObject()) {
        logger.log() << "Platform object not available";
        return false;
    }

    QJsonObject platformData = platformDataValue.toObject();

    QJsonValue latestValue = platformData.take("latest");
    if (!latestValue.isObject()) {
        logger.log() << "Platform.latest object not available";
        return false;
    }

    QJsonObject latestData = latestValue.toObject();

    QJsonValue latestVersionValue = latestData.take("version");
    if (!latestVersionValue.isString()) {
        logger.log() << "Platform.latest.version string not available";
        return false;
    }

    QJsonValue minimumValue = platformData.take("minimum");
    if (!minimumValue.isObject()) {
        logger.log() << "Platform.minimum object not available";
        return false;
    }

    QJsonObject minimumData = minimumValue.toObject();

    QJsonValue minimumVersionValue = minimumData.take("version");
    if (!minimumVersionValue.isString()) {
        logger.log() << "Platform.minimum.version string not available";
        return false;
    }

    double latestVersion = latestVersionValue.toString().toDouble();
    double minimumVersion = minimumVersionValue.toString().toDouble();
    double currentVersion = QString(APP_VERSION).toDouble();

    logger.log() << "Latest version:" << latestVersion;
    logger.log() << "Minimum version:" << minimumVersion;
    logger.log() << "Current version:" << currentVersion;

    if (currentVersion < minimumVersion) {
        logger.log() << "ReleaseMonitor - update required";
        MozillaVPN::instance()->setUpdateRecommended(false);
        MozillaVPN::instance()->controller()->updateRequired();
        return true;
    }

    logger.log() << "Update recommended: " << (currentVersion < latestVersion);
    MozillaVPN::instance()->setUpdateRecommended(currentVersion < latestVersion);
    return true;
}
