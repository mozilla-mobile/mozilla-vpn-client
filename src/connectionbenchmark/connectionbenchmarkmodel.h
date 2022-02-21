/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARKMODEL_H
#define CONNECTIONBENCHMARKMODEL_H

#include "connectionbenchmark.h"
#include "connectionbenchmarkdownload.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QList>
#include <QString>

class ConnectionBenchmarkModel final : public QAbstractListModel {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmarkModel);

  Q_PROPERTY(State state READ state NOTIFY stateChanged);

 public:
  ConnectionBenchmarkModel();
  ~ConnectionBenchmarkModel();

  enum State {
    StateInitial,
    StateTesting,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void reset();

  void addBenchmark();

  State state() const { return m_state; }

  enum ModelRoles { RoleBenchmark };

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void stateChanged();

 private:
  State m_state = StateInitial;

  void setState(State state);

  void initialize();

  QList<ConnectionBenchmark*> m_benchmarks;

  ConnectionBenchmarkDownload* m_benchmarkDownload;
};

#endif  // CONNECTIONBENCHMARKMODEL_H
