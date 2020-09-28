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
    Q_PROPERTY(quint64 txBytes READ txBytes NOTIFY bytesChanged)
    Q_PROPERTY(quint64 rxBytes READ rxBytes NOTIFY bytesChanged)

public:
    ConnectionDataHolder();

    void add(uint64_t txBytes, uint64_t rxBytes);

    Q_INVOKABLE void activate(const QVariant &txSeries,
                              const QVariant &rxSeries,
                              const QVariant &axisX,
                              const QVariant &axisY);

    Q_INVOKABLE void deactivate();

    const QString &ipAddress() const { return m_ipAddress; }

    void reset();

    quint64 txBytes() const;
    quint64 rxBytes() const;

    bool isRunning() const { return !!m_txSeries; }

private:
    void computeAxes();
    void updateIpAddress();

    // Boolean because we have just first and second index in the vector.
    quint64 bytes(bool index) const;

signals:
    void ipAddressChanged();
    void bytesChanged();

private:
    QtCharts::QSplineSeries *m_txSeries = nullptr;
    QtCharts::QSplineSeries *m_rxSeries = nullptr;
    QtCharts::QValueAxis *m_axisX = nullptr;
    QtCharts::QValueAxis *m_axisY = nullptr;

    QVector<QPair<uint64_t, uint64_t>> m_data;

    bool m_initialized = false;
    uint64_t m_txBytes = 0;
    uint64_t m_rxBytes = 0;
    uint64_t m_maxBytes = 0;

    QString m_ipAddress;
    QTimer m_ipAddressTimer;
};

#endif // CONNECTIONDATAHOLDER_H
