#ifndef CONNECTIONDATAHOLDER_H
#define CONNECTIONDATAHOLDER_H

#include <QObject>

namespace QtCharts {
class QSplineSeries;
class QValueAxis;
}

class ConnectionDataHolder : public QObject
{
    Q_OBJECT

public:
    void add(uint32_t txBytes, uint32_t rxBytes);

    Q_INVOKABLE void setComponents(const QVariant &series,
                                   const QVariant &axisX,
                                   const QVariant &axisY);

private:
    void computeAxes();

private:
    QtCharts::QSplineSeries *m_series = nullptr;
    QtCharts::QValueAxis *m_axisX = nullptr;
    QtCharts::QValueAxis *m_axisY = nullptr;
};

#endif // CONNECTIONDATAHOLDER_H
