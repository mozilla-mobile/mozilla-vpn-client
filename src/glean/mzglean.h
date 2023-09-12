/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MZGLEAN_H
#define MZGLEAN_H

#include <QObject>

constexpr const char* GLEAN_DATA_DIRECTORY = "glean";

class MZGlean final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MZGlean)

 private:
  explicit MZGlean(QObject* parent);

  static void setUploadEnabled(bool isTelemetryEnabled);

  /**
   * @brief Broadcast to the mobile VPN daemon instances of Glean,
   * that the telemetry uploadEnabled state has changed.
   *
   * @param isTelemetryEnabled The new upload enabled state.
   */
  static void broadcastUploadEnabledChange(bool isTelemetryEnabled);

 public:
  ~MZGlean();

  static void registerLogHandler(void (*messageHandler)(int32_t, char*));

  static void initialize();
  static void shutdown();
};

#endif  // MZGLEAN_H
