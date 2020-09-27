#include "connectiondataholder.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSplineSeries>
#include <QValueAxis>

constexpr int MAX_POINTS = 30;

// Let's refresh the IP address any 10 seconds.
constexpr int IPADDRESS_TIMER_MSEC = 10000;

ConnectionDataHolder::ConnectionDataHolder() : m_ipAddress(tr("Unknown"))
{
    emit ipAddressChanged();

    m_ipAddressTimer.start(IPADDRESS_TIMER_MSEC);
    connect(&m_ipAddressTimer, &QTimer::timeout, [this]() { updateIpAddress(); });
}

void ConnectionDataHolder::add(uint64_t txBytes, uint64_t rxBytes)
{
    qDebug() << "New connection data:" << txBytes << rxBytes;

    Q_ASSERT(!!m_txSeries == !!m_rxBytes);

    if (!m_txSeries) {
        return;
    }

    Q_ASSERT(m_txSeries->count() == MAX_POINTS);
    Q_ASSERT(m_rxSeries->count() == MAX_POINTS);

    // This is the first time we receive data. We need at least 2 calls in order to count the delta.
    if (m_initialized == false) {
        m_initialized = true;
        m_txBytes = txBytes;
        m_rxBytes = rxBytes;
        return;
    }

    // Normalize the value and store the previous max.
    uint64_t tmpTxBytes = txBytes;
    uint64_t tmpRxBytes = rxBytes;
    txBytes -= m_txBytes;
    rxBytes -= m_rxBytes;
    m_txBytes = tmpTxBytes;
    m_rxBytes = tmpRxBytes;

    m_maxBytes = std::max(m_maxBytes, std::max(txBytes, rxBytes));
    m_data.append(QPair(txBytes, rxBytes));

    while (m_data.length() > MAX_POINTS) {
        m_data.removeAt(0);
    }

    int i = 0;
    for (; i < MAX_POINTS - m_data.length(); ++i) {
        m_txSeries->replace(i, i, 0);
        m_rxSeries->replace(i, i, 0);
    }

    for (int j = 0; j < m_data.length(); ++j) {
        m_txSeries->replace(i, i, m_data.at(j).first);
        m_rxSeries->replace(i, i, m_data.at(j).second);
        ++i;
    }

    computeAxes();
}

void ConnectionDataHolder::setComponents(const QVariant &a_txSeries,
                                         const QVariant &a_rxSeries,
                                         const QVariant &a_axisX,
                                         const QVariant &a_axisY)
{
    qDebug() << "Set components";

    QtCharts::QSplineSeries *txSeries = qobject_cast<QtCharts::QSplineSeries *>(
        a_txSeries.value<QObject *>());

    if (m_txSeries != txSeries) {
        m_txSeries = txSeries;
    }

    QtCharts::QSplineSeries *rxSeries = qobject_cast<QtCharts::QSplineSeries *>(
        a_rxSeries.value<QObject *>());

    if (m_rxSeries != rxSeries) {
        m_rxSeries = rxSeries;
    }

    QtCharts::QValueAxis *axisX = qobject_cast<QtCharts::QValueAxis *>(a_axisX.value<QObject *>());

    if (m_axisX != axisX) {
        m_axisX = axisX;
    }

    QtCharts::QValueAxis *axisY = qobject_cast<QtCharts::QValueAxis *>(a_axisY.value<QObject *>());

    if (m_axisY != axisY) {
        m_axisY = axisY;
    }

    // Let's be sure we have all the x/y points.
    while (m_txSeries->count() < MAX_POINTS) {
        m_txSeries->append(m_txSeries->count(), 0);
        m_rxSeries->append(m_rxSeries->count(), 0);
    }
}

void ConnectionDataHolder::computeAxes()
{
    if (!m_axisX || !m_axisY) {
        return;
    }

    m_axisY->setRange(-1000, m_maxBytes * 1.5);
}

void ConnectionDataHolder::reset()
{
    qDebug() << "Resetting the data";

    m_initialized = false;
    m_txBytes = 0;
    m_rxBytes = 0;
    m_maxBytes = 0;
    m_data.clear();

    if (m_txSeries) {
        Q_ASSERT(m_rxSeries);
        Q_ASSERT(m_txSeries->count() == MAX_POINTS);
        Q_ASSERT(m_rxSeries->count() == MAX_POINTS);

        for (int i = 0; i < MAX_POINTS; ++i) {
            m_txSeries->replace(i, i, 0);
            m_rxSeries->replace(i, i, 0);
        }
    }

    updateIpAddress();
}

void ConnectionDataHolder::updateIpAddress()
{
    qDebug() << "Updating IP address";

    NetworkRequest *request = NetworkRequest::createForIpInfo(MozillaVPN::instance());
    connect(request, &NetworkRequest::requestFailed, [](QNetworkReply::NetworkError error) {
        qDebug() << "IP address request failed" << error;
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        qDebug() << "IP address request completed";

        QJsonDocument json = QJsonDocument::fromJson(data);
        Q_ASSERT(json.isObject());
        QJsonObject obj = json.object();

        Q_ASSERT(obj.contains("ip"));
        QJsonValue value = obj.take("ip");
        Q_ASSERT(value.isString());

        m_ipAddress = value.toString();
        emit ipAddressChanged();
    });
}
