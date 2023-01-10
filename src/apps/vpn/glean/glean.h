/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNGLEAN_H
#define VPNGLEAN_H

#include <QObject>

constexpr const char* GLEAN_DATA_DIRECTORY = "glean";

class VPNGlean final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(VPNGlean)

 private:
  explicit VPNGlean(QObject* parent);

  static void setUploadEnabled(bool isTelemetryEnabled);

 public:
  ~VPNGlean();

  static void registerLogHandler(void (*messageHandler)(int32_t, char*));

  static void initialize();
  static void shutdown();
};

#endif  // VPNGLEAN_H
