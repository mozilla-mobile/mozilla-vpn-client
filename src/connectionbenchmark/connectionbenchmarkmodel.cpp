/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"
#include "connectionbenchmarkmodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

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
  roles[RoleIcon] = "icon";
  roles[RoleName] = "name";
  roles[RoleResult] = "result";

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
    case RoleIcon:
      return "";
    case RoleName:
      return QVariant::fromValue(m_benchmarks.at(index.row()));
    case RoleResult:
      return "123 Mbits";
    default:
      return QVariant();
  }
}

void ConnectionBenchmarkModel::initialize() {
  m_benchmarks = {"one", "two", "three"};

  m_benchmarkss << new ConnectionBenchmark("id", "display name", true);
}