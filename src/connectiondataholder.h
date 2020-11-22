/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

#ifdef UNIT_TEST
class TestConnectionDataHolder;
#endif

class ConnectionDataHolder final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ConnectionDataHolder)

    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(quint64 txBytes READ txBytes NOTIFY bytesChanged)
    Q_PROPERTY(quint64 rxBytes READ rxBytes NOTIFY bytesChanged)

public:
    ConnectionDataHolder();
    ~ConnectionDataHolder();

    void enable();
    void disable();

    Q_INVOKABLE void activate(const QVariant &txSeries,
                              const QVariant &rxSeries,
                              const QVariant &axisX,
                              const QVariant &axisY);

    const QString &ipAddress() const { return m_ipAddress; }

    void reset();

    quint64 txBytes() const;
    quint64 rxBytes() const;

private:
    void add(uint64_t txBytes, uint64_t rxBytes);

    void computeAxes();
    void updateIpAddress();

    // Boolean because we have just first and second index in the vector.
    quint64 bytes(bool index) const;

public slots:
    Q_INVOKABLE void deactivate();

    void stateChanged();

signals:
    // for testing.
    void ipAddressChecked();

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

    bool m_updatingIpAddress = false;

    QString m_ipAddress;
    QTimer m_ipAddressTimer;
    QTimer m_checkStatusTimer;

#ifdef UNIT_TEST
    friend class TestConnectionDataHolder;
#endif
};

#endif // CONNECTIONDATAHOLDER_H
