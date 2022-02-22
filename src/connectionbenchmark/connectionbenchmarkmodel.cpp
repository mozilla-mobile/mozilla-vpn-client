/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkitem.h"
#include "connectionbenchmarkdownload.h"
#include "connectionbenchmarkmodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#include "qmlengineholder.h"

#include <QList>

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmarkModel");
}

ConnectionBenchmarkModel::ConnectionBenchmarkModel() {
  MVPN_COUNT_CTOR(ConnectionBenchmarkModel);
}

ConnectionBenchmarkModel::~ConnectionBenchmarkModel() {
  MVPN_COUNT_DTOR(ConnectionBenchmarkModel);
}

QHash<int, QByteArray> ConnectionBenchmarkModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[RoleBenchmark] = "benchmark";

  return roles;
}

int ConnectionBenchmarkModel::rowCount(const QModelIndex&) const {
  return m_benchmarks.count();
}

QVariant ConnectionBenchmarkModel::data(const QModelIndex& index,
                                        int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case RoleBenchmark:
      return QVariant::fromValue(m_benchmarks.at(index.row()));
    default:
      return QVariant();
  }
}

void ConnectionBenchmarkModel::setState(State state) {
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmarkModel::addResult(
    ConnectionBenchmarkItem* benchmarkItem) {
  logger.debug() << "Add benchmark result";

  beginInsertRows(QModelIndex(), m_benchmarks.size(), m_benchmarks.size());
  m_benchmarks.append(benchmarkItem);
  endInsertRows();
}

void ConnectionBenchmarkModel::runNextBenchmark() {
  if (m_state == StatePing) {
    logger.debug() << "Run ping benchmark";

    addResult(new ConnectionBenchmarkItem("ping", "Ping", 15));

    setState(StateDownload);
    runNextBenchmark();

  } else if (m_state == StateDownload) {
    logger.debug() << "Run download benchmark";

    m_benchmarkDownload = new ConnectionBenchmarkDownload();
    connect(
        m_benchmarkDownload, &ConnectionBenchmarkDownload::stateChanged, this,
        [&] {
          logger.debug() << "Download speedtest state changed";

          if (m_benchmarkDownload->state() ==
              ConnectionBenchmarkDownload::StateReady) {
            addResult(new ConnectionBenchmarkItem(
                "download", "Download", m_benchmarkDownload->downloadSpeed()));
            setState(StateReady);

          } else if (m_benchmarkDownload->state() ==
                     ConnectionBenchmarkDownload::StateError) {
            setState(StateError);
          }
        });

    m_benchmarkDownload->start();
  }
}

void ConnectionBenchmarkModel::start() {
  logger.debug() << "Start connection benchmarking";

  if (m_benchmarks.size() > 0) {
    reset();
  }

  setState(StatePing);
  runNextBenchmark();
}

void ConnectionBenchmarkModel::stop() { logger.debug() << "Stop benchmark"; }

void ConnectionBenchmarkModel::reset() {
  logger.debug() << "Reset connection benchmarks";

  m_benchmarks.clear();
  setState(StateInitial);
}