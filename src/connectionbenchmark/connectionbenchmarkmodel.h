/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARKMODEL_H
#define CONNECTIONBENCHMARKMODEL_H

#include "connectionbenchmark.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QList>
#include <QString>

class ConnectionBenchmarkModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionBenchmarkModel)

 public:
  ConnectionBenchmarkModel();
  ~ConnectionBenchmarkModel();

  enum ModelRoles {
    RoleName,
    RoleResult,
    RoleIcon,
  };

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:

 private:
  void initialize();

  QList<QString> m_benchmarks;

  QList<ConnectionBenchmark*> m_benchmarkss;
};

#endif  // CONNECTIONBENCHMARKMODEL_H
