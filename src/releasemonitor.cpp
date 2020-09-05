#include "releasemonitor.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <QTimer>
#include <QtDebug>

// Any 6 hours, a new check
constexpr uint32_t RELEASE_MONITOR_SEC = 21600;

constexpr const char *LATEST_VERSION_KEY = "latestVersion";
constexpr const char *MINIMUM_VERSION_KEY = "minimumVersion";

void ReleaseMonitor::init(MozillaVPN *vpn, QSettings *settings)
{
    m_vpn = vpn;
    m_settings = settings;

    if (m_settings->contains(LATEST_VERSION_KEY)) {
        m_latestVersion = m_settings->value(LATEST_VERSION_KEY).toDouble();
    }

    if (m_settings->contains(MINIMUM_VERSION_KEY)) {
        m_latestVersion = m_settings->value(MINIMUM_VERSION_KEY).toDouble();
    }

    maybeForceUpdate();
}

void ReleaseMonitor::run()
{
    qDebug() << "ReleaseMonitor started";

    NetworkRequest *request = NetworkRequest::createForVersions(m_vpn);

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
    QTimer::singleShot(RELEASE_MONITOR_SEC * 1000, [this] { run(); });
}

void ReleaseMonitor::processData(const QByteArray &data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    Q_ASSERT(json.isObject());
    QJsonObject obj = json.object();

    QString platformKey =
#ifdef __APPLE__
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

    m_latestVersion = latestVersionValue.toString().toDouble();
    m_minimumVersion = minimumVersionValue.toString().toDouble();

    m_settings->setValue(LATEST_VERSION_KEY, m_latestVersion);
    m_settings->setValue(MINIMUM_VERSION_KEY, m_minimumVersion);

    maybeForceUpdate();
}

void ReleaseMonitor::maybeForceUpdate()
{
    double currentVersion = QString(APP_VERSION).toDouble();

    qDebug() << "Latest version:" << m_latestVersion;
    qDebug() << "Minimum version:" << m_minimumVersion;
    qDebug() << "Current version:" << currentVersion;

    if (currentVersion < m_minimumVersion) {
        m_vpn->forceUpdateState();
        return;
    }

    m_vpn->setUpdateRecommended(currentVersion < m_latestVersion);
}
