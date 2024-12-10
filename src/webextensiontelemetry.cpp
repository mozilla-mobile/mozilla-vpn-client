/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webextensiontelemetry.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>

#include "glean/boolean.h"
#include "glean/event.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "glean/quantity.h"
#include "logger.h"

namespace {
Logger logger("WebExtensionTelemetry");
}  // namespace

namespace WebextensionTelemetry {

std::optional<TelemetryInfo> fromJson(const QJsonObject& obj) {
  // 1: Needs to have type:"telemetry"
  if (obj["t"].toString("") != "telemetry") {
    return {};
  }
  // 2: MUST have name
  TelemetryInfo out{};
  if (!obj["name"].isString()) {
    return {};
  }
  out.name = obj["name"].toString();
  // 3: Can have Variant args
  if (obj["args"].toVariant().isValid()) {
    out.args = obj["args"].toVariant();
  }
  return out;
}
// Const functions to create handlers for glean data types.
namespace handlers {
// For Events we want to just record it to glean.
// It will be send out with the next ping
auto constexpr event(EventMetric* metric) {
  return [metric](QVariant) { metric->record(); };
};
auto constexpr count(QuantityMetric* metric) {
  return [metric](QVariant args) {
    bool ok;
    const auto count = args.toUInt(&ok);
    if (!ok) {
      return;
    }
    metric->set(count);
  };
};
// For flags, we will flip the flag-bit in the settings to true.
// We will also record this to glean.
// Flags will always be sent with the next ping
auto constexpr flag(BooleanMetric* metric) {
  return [metric](QVariant) { metric->set(true); };
}
using namespace mozilla::glean;
const auto map = QMap<QString, std::function<void(QVariant)>>{
    {"fx_protection_disabled", event(&extension::fx_protection_disabled)},
    {"fx_protection_enabled", event(&extension::fx_protection_enabled)},
    {"fx_protection_mode_changed",
     event(&extension::fx_protection_mode_changed)},
    {"main_screen", event(&extension::main_screen)},
    {"error_screen", event(&mozilla::glean::extension::error_screen)},
    {"has_completed_onboarding", flag(&extension::has_completed_onboarding)},
    {"used_feature_disable_firefox_protection",
     flag(&extension::used_feature_disable_firefox_protection)},
    {"used_feature_settings_page",
     flag(&extension::used_feature_settings_page)},
    {"used_feature_page_action_revoke_exclude",
     flag(&extension::used_feature_page_action_revoke_exclude)},
    {"used_feature_page_action_revoke_geopref",
     flag(&extension::used_feature_page_action_revoke_geopref)},
    {"count_excluded", count(&extension::count_excluded)},
    {"count_geoprefed", count(&extension::count_geoprefed)},
};
}  // namespace handlers

bool recordTelemetry(const TelemetryInfo& info) {
  if (!handlers::map.contains(info.name)) {
    return false;
  }
  auto handler = handlers::map[info.name];
  handler(info.args);
  return true;
}
void startSession() {
  mozilla::glean_pings::Extensionsession.submit("extension_start");
}
void stopSession() {
  mozilla::glean_pings::Extensionsession.submit("extension_stop");
}
}  // namespace WebextensionTelemetry
