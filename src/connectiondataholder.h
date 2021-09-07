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

#if QT_VERSION >= 0x060000
class QSplineSeries;
class QValueAxis;
#else
namespace QtCharts {
class QSplineSeries;
class QValueAxis;
}  // namespace QtCharts

using namespace QtCharts;
#endif

#ifdef UNIT_TEST
class TestConnectionDataHolder;
#endif

class ConnectionDataHolder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionDataHolder)

  Q_PROPERTY(QString ipv4Address READ ipv4Address NOTIFY ipv4AddressChanged)
  Q_PROPERTY(QString ipv6Address READ ipv6Address NOTIFY ipv6AddressChanged)
  Q_PROPERTY(quint64 txBytes READ txBytes NOTIFY bytesChanged)
  Q_PROPERTY(quint64 rxBytes READ rxBytes NOTIFY bytesChanged)

 public:
  ConnectionDataHolder();
  ~ConnectionDataHolder();

  void enable();
  void disable();

  Q_INVOKABLE void activate(const QVariant& txSeries, const QVariant& rxSeries,
                            const QVariant& axisX, const QVariant& axisY);

  const QString& ipv4Address() const { return m_ipv4Address; }
  const QString& ipv6Address() const { return m_ipv6Address; }

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

  void ipv4AddressChanged();
  void ipv6AddressChanged();
  void bytesChanged();

 private:
  QSplineSeries* m_txSeries = nullptr;
  QSplineSeries* m_rxSeries = nullptr;
  QValueAxis* m_axisX = nullptr;
  QValueAxis* m_axisY = nullptr;

  QVector<QPair<uint64_t, uint64_t>> m_data;

  bool m_initialized = false;
  uint64_t m_txBytes = 0;
  uint64_t m_rxBytes = 0;
  uint64_t m_maxBytes = 0;

  bool m_updatingIpAddress = false;

  QString m_ipv4Address;
  QString m_ipv6Address;
  QTimer m_ipAddressTimer;
  QTimer m_checkStatusTimer;

#ifdef UNIT_TEST
  friend class TestConnectionDataHolder;
#endif
};

#endif  // CONNECTIONDATAHOLDER_H
