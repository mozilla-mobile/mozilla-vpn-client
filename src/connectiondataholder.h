#ifndef CONNECTIONDATAHOLDER_H
#define CONNECTIONDATAHOLDER_H

#include <QObject>
#include <QPair>
#include <QString>
#include <QTimer>
#include <QVector>

namespace QtCharts {
class QSplineSeries;
class QValueAxis;
}

class ConnectionDataHolder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY ipAddressChanged)

public:
    ConnectionDataHolder();

    void add(uint32_t txBytes, uint32_t rxBytes);

    Q_INVOKABLE void setComponents(const QVariant &txSeries,
                                   const QVariant &rxSeries,
                                   const QVariant &axisX,
                                   const QVariant &axisY);

    const QString &ipAddress() const { return m_ipAddress; }

    void reset();

private:
    void computeAxes();
    void updateIpAddress();

signals:
    void ipAddressChanged();

private:
    QtCharts::QSplineSeries *m_txSeries = nullptr;
    QtCharts::QSplineSeries *m_rxSeries = nullptr;
    QtCharts::QValueAxis *m_axisX = nullptr;
    QtCharts::QValueAxis *m_axisY = nullptr;

    QVector<QPair<uint32_t, uint32_t>> m_data;

    int64_t m_txBytes = -1;
    int64_t m_rxBytes = -1;
    uint32_t m_maxBytes = 0;

    QString m_ipAddress;
    QTimer m_ipAddressTimer;
};

#endif // CONNECTIONDATAHOLDER_H
