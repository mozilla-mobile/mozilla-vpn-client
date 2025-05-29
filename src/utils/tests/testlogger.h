/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class TestLogger final : public QObject {
  Q_OBJECT

 public:
  Q_INVOKABLE TestLogger(QObject* parent = nullptr) : QObject(parent) {};

 private slots:
  void logger();

  void logHandler();

  void logTruncation();
};
