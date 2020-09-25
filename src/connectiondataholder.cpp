#include "connectiondataholder.h"

#include <QDebug>
#include <QSplineSeries>
#include <QValueAxis>

constexpr int MAX_POINTS = 30;

void ConnectionDataHolder::add(uint32_t txBytes, uint32_t rxBytes)
{
    qDebug() << "New connection data:" << txBytes << rxBytes;

    if (!m_txSeries || !m_rxSeries) {
        return;
    }

    if (m_data.length() > MAX_POINTS) {
        m_data.removeAt(0);
    }

    m_data.append(QPair(txBytes, rxBytes));

    // Let's be sure we have all the x/y points.
    while (m_txSeries->count() <= MAX_POINTS) {
        m_txSeries->append(m_txSeries->count(), 0);
        m_rxSeries->append(m_rxSeries->count(), 0);
    }

    int i = 0;
    for (; i < MAX_POINTS - m_data.length(); ++i) {
        m_txSeries->replace(i, i, 0);
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
        if (m_txSeries && m_txSeries->parent() == this) {
            delete m_txSeries;
        }

        m_txSeries = txSeries;
    }

    QtCharts::QSplineSeries *rxSeries = qobject_cast<QtCharts::QSplineSeries *>(
        a_rxSeries.value<QObject *>());

    if (m_rxSeries != rxSeries) {
        if (m_rxSeries && m_rxSeries->parent() == this) {
            delete m_rxSeries;
        }

        m_rxSeries = rxSeries;
    }

    QtCharts::QValueAxis *axisX = qobject_cast<QtCharts::QValueAxis *>(a_axisX.value<QObject *>());

    if (m_axisX != axisX) {
        if (m_axisX && m_axisX->parent() == this) {
            delete m_axisX;
        }

        m_axisX = axisX;
    }

    QtCharts::QValueAxis *axisY = qobject_cast<QtCharts::QValueAxis *>(a_axisY.value<QObject *>());

    if (m_axisY != axisY) {
        if (m_axisY && m_axisY->parent() == this) {
            delete m_axisY;
        }

        m_axisY = axisY;
    }
}

void ConnectionDataHolder::computeAxes()
{
    qDebug() << "Computing axes";
    if (!m_axisX || !m_axisY) {
        return;
    }

    //m_axisX->setRange(m_rxSeries->count() - MAX_POINTS, m_rxSeries->count());
}
