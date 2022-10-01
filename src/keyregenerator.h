/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef KEYREGENERATOR_H
#define KEYREGENERATOR_H

#include <QObject>
#include <QTimer>

class KeyRegenerator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(KeyRegenerator)

 public:
  KeyRegenerator();
  ~KeyRegenerator();

 private:
  void stateChanged();

 private:
  QTimer m_timer;
};

#endif  // KEYREGENERATOR_H
