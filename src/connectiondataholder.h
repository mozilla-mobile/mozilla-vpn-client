#ifndef CONNECTIONDATAHOLDER_H
#define CONNECTIONDATAHOLDER_H

#include <QObject>
#include <QPair>
#include <QVector>

namespace QtCharts {
class QSplineSeries;
class QValueAxis;
}

class ConnectionDataHolder : public QObject
{
    Q_OBJECT

public:
    void add(uint32_t txBytes, uint32_t rxBytes);

    Q_INVOKABLE void setComponents(const QVariant &txSeries,
                                   const QVariant &rxSeries,
                                   const QVariant &axisX,
                                   const QVariant &axisY);

private:
    void computeAxes();

private:
    QtCharts::QSplineSeries *m_txSeries = nullptr;
    QtCharts::QSplineSeries *m_rxSeries = nullptr;
    QtCharts::QValueAxis *m_axisX = nullptr;
    QtCharts::QValueAxis *m_axisY = nullptr;

    QVector<QPair<uint32_t, uint32_t>> m_data;
};

#endif // CONNECTIONDATAHOLDER_H
