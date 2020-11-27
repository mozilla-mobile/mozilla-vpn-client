/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectiondataholder.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSplineSeries>
#include <QValueAxis>

namespace {
Logger logger(LOG_NETWORKING, "ConnectionDataHolder");
}

//% "Unknown"
//: This refers to the current IP address, i.e. "IP: Unknown".
ConnectionDataHolder::ConnectionDataHolder()
    : m_ipAddress(qtTrId("vpn.connectionInfo.unknown")) {
  MVPN_COUNT_CTOR(ConnectionDataHolder);

  connect(&m_ipAddressTimer, &QTimer::timeout, [this]() { updateIpAddress(); });
  connect(&m_checkStatusTimer, &QTimer::timeout, [this]() {
    MozillaVPN::instance()->controller()->getStatus(
        [this](const QString& serverIpv4Gateway, uint64_t txBytes,
               uint64_t rxBytes) {
          if (!serverIpv4Gateway.isEmpty()) {
            add(txBytes, rxBytes);
          }
        });
  });
}

ConnectionDataHolder::~ConnectionDataHolder() {
  MVPN_COUNT_DTOR(ConnectionDataHolder);
}

void ConnectionDataHolder::enable() {
  m_ipAddressTimer.start(Constants::IPADDRESS_TIMER_MSEC);
}

void ConnectionDataHolder::disable() { m_ipAddressTimer.stop(); }

void ConnectionDataHolder::add(uint64_t txBytes, uint64_t rxBytes) {
  logger.log() << "New connection data:" << txBytes << rxBytes;

  Q_ASSERT(!!m_txSeries == !!m_rxSeries);

  if (!m_txSeries) {
    return;
  }

  Q_ASSERT(m_txSeries->count() == Constants::CHARTS_MAX_POINTS);
  Q_ASSERT(m_rxSeries->count() == Constants::CHARTS_MAX_POINTS);

  // This is the first time we receive data. We need at least 2 calls in order
  // to count the delta.
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
  m_data.append(QPair<uint64_t, uint64_t>(txBytes, rxBytes));

  while (m_data.length() > Constants::CHARTS_MAX_POINTS) {
    m_data.removeAt(0);
  }

  int i = 0;
  for (; i < Constants::CHARTS_MAX_POINTS - m_data.length(); ++i) {
    m_txSeries->replace(i, i, 0);
    m_rxSeries->replace(i, i, 0);
  }

  for (int j = 0; j < m_data.length(); ++j) {
    m_txSeries->replace(i, i, m_data.at(j).first);
    m_rxSeries->replace(i, i, m_data.at(j).second);
    ++i;
  }

  computeAxes();
  emit bytesChanged();
}

void ConnectionDataHolder::activate(const QVariant& a_txSeries,
                                    const QVariant& a_rxSeries,
                                    const QVariant& a_axisX,
                                    const QVariant& a_axisY) {
  logger.log() << "Activated";

  QtCharts::QSplineSeries* txSeries =
      qobject_cast<QtCharts::QSplineSeries*>(a_txSeries.value<QObject*>());
  Q_ASSERT(txSeries);

  if (m_txSeries != txSeries) {
    m_txSeries = txSeries;
    connect(txSeries, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  QtCharts::QSplineSeries* rxSeries =
      qobject_cast<QtCharts::QSplineSeries*>(a_rxSeries.value<QObject*>());
  Q_ASSERT(rxSeries);

  if (m_rxSeries != rxSeries) {
    m_rxSeries = rxSeries;
    connect(rxSeries, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  QtCharts::QValueAxis* axisX =
      qobject_cast<QtCharts::QValueAxis*>(a_axisX.value<QObject*>());
  Q_ASSERT(axisX);

  if (m_axisX != axisX) {
    m_axisX = axisX;
    connect(axisX, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  QtCharts::QValueAxis* axisY =
      qobject_cast<QtCharts::QValueAxis*>(a_axisY.value<QObject*>());
  Q_ASSERT(axisY);

  if (m_axisY != axisY) {
    m_axisY = axisY;
    connect(axisY, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  // Let's be sure we have all the x/y points.
  while (m_txSeries->count() < Constants::CHARTS_MAX_POINTS) {
    m_txSeries->append(m_txSeries->count(), 0);
    m_rxSeries->append(m_rxSeries->count(), 0);
  }

  m_checkStatusTimer.start(Constants::CHECKSTATUS_TIMER_MSEC);
}

void ConnectionDataHolder::deactivate() {
  logger.log() << "Deactivated";

  reset();
  m_axisX = nullptr;
  m_axisY = nullptr;
  m_txSeries = nullptr;
  m_rxSeries = nullptr;

  m_checkStatusTimer.stop();
}

void ConnectionDataHolder::computeAxes() {
  if (!m_axisX || !m_axisY) {
    return;
  }

  m_axisY->setRange(-1000, m_maxBytes * 1.5);
}

void ConnectionDataHolder::reset() {
  logger.log() << "Resetting the data";

  m_initialized = false;
  m_txBytes = 0;
  m_rxBytes = 0;
  m_maxBytes = 0;
  m_data.clear();

  emit bytesChanged();

  if (m_txSeries) {
    Q_ASSERT(m_txSeries->count() == Constants::CHARTS_MAX_POINTS);
    Q_ASSERT(m_rxSeries->count() == Constants::CHARTS_MAX_POINTS);

    for (int i = 0; i < Constants::CHARTS_MAX_POINTS; ++i) {
      m_txSeries->replace(i, i, 0);
      m_rxSeries->replace(i, i, 0);
    }
  }

  updateIpAddress();
}

void ConnectionDataHolder::updateIpAddress() {
  logger.log() << "Updating IP address";

  if (m_updatingIpAddress) {
    return;
  }
  m_updatingIpAddress = true;

  NetworkRequest* request = NetworkRequest::createForIpInfo(this);
  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error) {
            logger.log() << "IP address request failed" << error;

            ErrorHandler::ErrorType errorType =
                ErrorHandler::toErrorType(error);
            if (errorType == ErrorHandler::AuthenticationError) {
              MozillaVPN::instance()->errorHandle(errorType);
            }

            m_updatingIpAddress = false;
            emit ipAddressChecked();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.log() << "IP address request completed";

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isObject()) {
              QJsonObject obj = json.object();

              QJsonValue value = obj.take("ip");
              if (value.isString()) {
                m_ipAddress = value.toString();
                emit ipAddressChanged();
              }
            }

            m_updatingIpAddress = false;
            emit ipAddressChecked();
          });
}

quint64 ConnectionDataHolder::txBytes() const { return bytes(0); }

quint64 ConnectionDataHolder::rxBytes() const { return bytes(1); }

quint64 ConnectionDataHolder::bytes(bool index) const {
  if (m_data.isEmpty()) {
    return 0;
  }

  const QPair<uint64_t, uint64_t>& pair = m_data.last();
  return !index ? pair.first : pair.second;
}

void ConnectionDataHolder::stateChanged() {
  logger.log() << "state changed";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->state() != MozillaVPN::StateMain) {
    disable();
    return;
  }

  enable();

  reset();

  if (m_txSeries && vpn->controller()->state() == Controller::StateOn) {
    m_checkStatusTimer.start();
  }
}
