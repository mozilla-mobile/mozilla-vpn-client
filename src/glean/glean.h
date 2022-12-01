/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NEW_GLEAN_H
#define NEW_GLEAN_H

#include <QObject>

constexpr const char* GLEAN_DATA_DIRECTORY = "glean";

class VPNGlean final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(VPNGlean)

 private:
  VPNGlean();

  static void setUploadEnabled(bool isTelemetryEnabled);

 public:
  ~VPNGlean();

  static void initialize();

  // TODO: Just use the glean_core type once
  // https://github.com/mozilla/glean/pull/2283 lands.
  enum ErrorType {
    /// For when the value to be recorded does not match the metric-specific
    /// restrictions
    InvalidValue,
    /// For when the label of a labeled metric does not match the restrictions
    InvalidLabel,
    /// For when the metric caught an invalid state while recording
    InvalidState,
    /// For when the value to be recorded overflows the metric-specific upper
    /// range
    InvalidOverflow,
  };
};

#endif  // NEW_GLEAN_H
