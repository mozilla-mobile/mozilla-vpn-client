#include "connectiondataholder.h"

#include <QDebug>
#include <QSplineSeries>
#include <QValueAxis>

static uint32_t todo = 0;

void ConnectionDataHolder::add(uint32_t txBytes, uint32_t rxBytes)
{
    qDebug() << "New connection data:" << txBytes << rxBytes << m_series;

    if (!m_series) {
        return;
    }

    m_series->append(++todo, (txBytes + rxBytes) / 1024);

    computeAxes();
}

void ConnectionDataHolder::setComponents(const QVariant &a_series,
                                         const QVariant &a_axisX,
                                         const QVariant &a_axisY)
{
    qDebug() << "Set components";

    QtCharts::QSplineSeries *series = qobject_cast<QtCharts::QSplineSeries *>(
        a_series.value<QObject *>());

    if (m_series != series) {
        if (m_series && m_series->parent() == this) {
            delete m_series;
        }

        m_series = series;
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
}
