/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectiondataholder.h"
#include "constants.h"
#include "ipfinder.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "timersingleshot.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSplineSeries>
#include <QValueAxis>

namespace {
Logger logger(LOG_NETWORKING, "ConnectionDataHolder");
}

ConnectionDataHolder::ConnectionDataHolder()
    //% "Loading"
    //: This refers to the current IP address, i.e. "IP: Loading".
    : m_ipv4Address(qtTrId("vpn.connectionInfo.loading")),
      m_ipv6Address(qtTrId("vpn.connectionInfo.loading")) {
  MVPN_COUNT_CTOR(ConnectionDataHolder);

  connect(&m_ipAddressTimer, &QTimer::timeout, [this]() { updateIpAddress(); });
  connect(&m_checkStatusTimer, &QTimer::timeout, [this]() {
    MozillaVPN::instance()->controller()->getStatus(
        [this](const QString& serverIpv4Gateway,
               const QString& deviceIpv4Address, uint64_t txBytes,
               uint64_t rxBytes) {
          Q_UNUSED(deviceIpv4Address);
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
  m_ipAddressTimer.start(Constants::ipAddressTimerMsec());
}

void ConnectionDataHolder::disable() { m_ipAddressTimer.stop(); }

void ConnectionDataHolder::add(uint64_t txBytes, uint64_t rxBytes) {
  logger.debug() << "New connection data:" << txBytes << rxBytes;

  Q_ASSERT(!!m_txSeries == !!m_rxSeries);

  if (!m_txSeries) {
    return;
  }

  Q_ASSERT(m_txSeries->count() == Constants::chartsMaxPoints());
  Q_ASSERT(m_rxSeries->count() == Constants::chartsMaxPoints());

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

  while (m_data.length() > Constants::chartsMaxPoints()) {
    m_data.removeAt(0);
  }

  int i = 0;
  for (; i < Constants::chartsMaxPoints() - m_data.length(); ++i) {
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
  logger.info() << "Activated";
  updateIpAddress();

  QSplineSeries* txSeries =
      qobject_cast<QSplineSeries*>(a_txSeries.value<QObject*>());
  Q_ASSERT(txSeries);

  if (m_txSeries != txSeries) {
    m_txSeries = txSeries;
    connect(txSeries, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  QSplineSeries* rxSeries =
      qobject_cast<QSplineSeries*>(a_rxSeries.value<QObject*>());
  Q_ASSERT(rxSeries);

  if (m_rxSeries != rxSeries) {
    m_rxSeries = rxSeries;
    connect(rxSeries, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  QValueAxis* axisX = qobject_cast<QValueAxis*>(a_axisX.value<QObject*>());
  Q_ASSERT(axisX);

  if (m_axisX != axisX) {
    m_axisX = axisX;
    connect(axisX, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  QValueAxis* axisY = qobject_cast<QValueAxis*>(a_axisY.value<QObject*>());
  Q_ASSERT(axisY);

  if (m_axisY != axisY) {
    m_axisY = axisY;
    connect(axisY, &QObject::destroyed, this,
            &ConnectionDataHolder::deactivate);
  }

  // Let's be sure we have all the x/y points.
  while (m_txSeries->count() < Constants::chartsMaxPoints()) {
    m_txSeries->append(m_txSeries->count(), 0);
    m_rxSeries->append(m_rxSeries->count(), 0);
  }

  m_checkStatusTimer.start(Constants::checkStatusTimerMsec());
}

void ConnectionDataHolder::deactivate() {
  logger.info() << "Deactivated";

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
  logger.debug() << "Resetting the data";

  m_initialized = false;
  m_txBytes = 0;
  m_rxBytes = 0;
  m_maxBytes = 0;
  m_data.clear();

  emit bytesChanged();

  if (m_txSeries) {
    // In theory, m_txSeries and m_rxSeries should contain
    // Constants::CHARTS_MAX_POINTS elements, but during the shutdown, strange
    // things happen.
    for (int i = 0; i < m_txSeries->count(); ++i) {
      m_txSeries->replace(i, i, 0);
    }

    for (int i = 0; i < m_rxSeries->count(); ++i) {
      m_rxSeries->replace(i, i, 0);
    }
  }

  updateIpAddress();
}

void ConnectionDataHolder::updateIpAddress() {
  logger.debug() << "Updating IP address";
  auto state = MozillaVPN::instance()->controller()->state();
  // Only start the check if we're actually connected/connecting
  if (state != Controller::StateOn && state != Controller::StateConfirming) {
    logger.warning() << "Skip Updating IP address, not connected";
    return;
  }

  if (m_updatingIpAddress) {
    return;
  }
  m_updatingIpAddress = true;

  IPFinder* ipfinder = new IPFinder(this);
  connect(
      ipfinder, &IPFinder::completed,
      [this](const QString& ipv4, const QString& ipv6, const QString& country) {
        if (ipv4.isEmpty() && ipv6.isEmpty()) {
          logger.error() << "IP address request failed";
          m_updatingIpAddress = false;
          emit ipAddressChecked();
          return;
        }

        MozillaVPN* vpn = MozillaVPN::instance();
        logger.debug() << "IP address request completed";
        if (m_checkStatusTimer.isActive() &&
            country != vpn->currentServer()->exitCountryCode()) {
          // In case the country-we're reported in does not match the
          // connected server we may retry only once.
          logger.warning() << "Reported ip not in the right country, retry!";
          TimerSingleShot::create(this, 3000, [this]() { updateIpAddress(); });
        }

        if (!ipv4.isEmpty()) {
          m_ipv4Address = ipv4;
          emit ipv4AddressChanged();
        }

        if (!ipv4.isEmpty()) {
          m_ipv6Address = ipv6;
          emit ipv6AddressChanged();
        }

        logger.debug() << "Set own Address. ipv4:"
                       << logger.sensitive(m_ipv4Address)
                       << "ipv6:" << logger.sensitive(m_ipv6Address) << "in"
                       << logger.sensitive(country);

        m_updatingIpAddress = false;
        emit ipAddressChecked();
      });

  ipfinder->start();
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
  logger.debug() << "state changed";

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
