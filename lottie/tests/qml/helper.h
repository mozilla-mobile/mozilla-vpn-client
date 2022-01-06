/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottie.h"

#include <QObject>

class Setup final : public QObject {
  Q_OBJECT

 public:
  Setup() = default;
  ~Setup() = default;

 public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    Lottie::initialize(engine, "LottieTest 0.1");
  }
};
