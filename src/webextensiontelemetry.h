/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QJSONObject>
#include <QString>
#include <QVariant>
#include <optional>

namespace WebextensionTelemetry {
struct TelemetryInfo {
  QString name;
  QVariant args;
};

/**
 * @brief Tries to parse a JSON Blob into a telemetry info obj,
 * @return std::optional<TelemetryInfo> - Nullopt on parse-error.
 */
std::optional<TelemetryInfo> fromJson(const QJsonObject& obj);
/**
 * @brief Records telemetry to glean
 *
 * @param info - The Info to record, create with fromJson();
 * @return true - info was accepted
 * @return false - info was rejected
 */
bool recordTelemetry(const TelemetryInfo& info);

void startSession();
void stopSession();

};  // namespace WebextensionTelemetry
