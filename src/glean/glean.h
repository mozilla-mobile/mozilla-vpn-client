/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GLEAN_H
#define GLEAN_H

#include <QObject>

constexpr const char* GLEAN_DATA_DIRECTORY = "glean";

class Glean final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Glean)

 private:
  Glean();

  static void setUploadEnabled(bool isTelemetryEnabled);

 public:
  ~Glean();

  static void initialize();
};

#endif  // GLEAN_H
