/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BPFSPLITTUNNEL_H
#define BPFSPLITTUNNEL_H

#include <QObject>
#include <QDir>

class BpfSplitTunnel final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BpfSplitTunnel)

 public:
  explicit BpfSplitTunnel(QObject* parent = nullptr);
  ~BpfSplitTunnel();

 public:
  void attachCgroup(const QString& cgroup);
  void detachCgroup(const QString& cgroup);
  void detachAll();

 private:
  int loadProgram();

 private:
  int m_program = -1;
  QString m_cgroupMount;
};

#endif  // BPFSPLITTUNNEL_H
