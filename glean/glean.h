/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GLEAN_H
#define GLEAN_H

#include <QQmlEngine>

class Glean final {
 private:
  Glean() = default;

 public:
  static void Initialize(QQmlEngine* engine);
};

#endif  // GLEAN_H
