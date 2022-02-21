/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"
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

  ConnectionBenchmarkModel::initialize();
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

void ConnectionBenchmarkModel::addBenchmark() {
  logger.debug() << "Add benchmark";

  m_benchmarkDownload = new ConnectionBenchmarkDownload();

  connect(m_benchmarkDownload, &ConnectionBenchmarkDownload::stateChanged, this,
          [&] {
            logger.debug() << "Download speedtest state changed";

            if (m_benchmarkDownload->state() ==
                ConnectionBenchmarkDownload::StateReady) {
              m_benchmarks = {new ConnectionBenchmark(
                  "download", "Download 2",
                  m_benchmarkDownload->downloadSpeed(), true)};

              emit dataChanged(createIndex(0, 0),
                               createIndex(m_benchmarks.size(), 0));

              setState(StateReady);
            } else if (m_benchmarkDownload->state() ==
                       ConnectionBenchmarkDownload::StateError) {
              setState(StateError);
            }
          });

  m_benchmarkDownload->start();
}

void ConnectionBenchmarkModel::start() {
  logger.debug() << "Start benchmark";

  setState(StateTesting);
  addBenchmark();
}

void ConnectionBenchmarkModel::stop() { logger.debug() << "Stop benchmark"; }

void ConnectionBenchmarkModel::reset() {
  logger.debug() << "Reset benchmark";

  m_benchmarks.clear();
  setState(StateInitial);
}

void ConnectionBenchmarkModel::initialize() {
  m_benchmarks = {new ConnectionBenchmark("download", "Download 1", 0, true)};
}
