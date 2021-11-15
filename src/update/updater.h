/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class Updater : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Updater)

 public:
  static Updater* create(QObject* parent, bool downloadAndInstall);

  Updater(QObject* parent);
  virtual ~Updater();

  virtual void start() = 0;

 signals:
  void updateRequired();
  void updateRecommended();
  void updateNotAvailable();

 protected:
  static QString appVersion();
};

#endif  // UPDATER_H
