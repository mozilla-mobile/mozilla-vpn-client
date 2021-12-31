/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOTTIE_H
#define LOTTIE_H

#include <QQmlEngine>

class Lottie final {
 private:
  Lottie() = default;

 public:
  static void initialize(QQmlEngine* engine, const QString& userAgent);
};

#endif  // LOTTIE_H
