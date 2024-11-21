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
#include "settingsholder.h"

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
template <uint8_t bit>  // The bit in the bitfield to denote
auto consteval flag(BooleanMetric* metric) {
  static_assert(bit < 32u,
                "The Bitfield is a u32, cannout assign a bigger bitfield");
  return [metric](QVariant) {
    auto settings = SettingsHolder::instance();
    uint32_t currentBitFlags = settings->extensionTelemetryFlags();
    // Flip the bit
    uint32_t newFlags = currentBitFlags | (1 << bit);
    if (currentBitFlags != newFlags) {
      // Avoid disk io.
      settings->setExtensionTelemetryFlags(currentBitFlags);
    }
    metric->set(true);
  };
}
using namespace mozilla::glean;
const auto map = QMap<QString, std::function<void(QVariant)>>{
    {"fx_protection_disenabled", event(&extension::fx_protection_disenabled)},
    {"fx_protection_enabled", event(&extension::fx_protection_enabled)},
    {"fx_protection_mode_changed",
     event(&extension::fx_protection_mode_changed)},
    {"main_screen", event(&extension::main_screen)},
    {"error_screen", event(&mozilla::glean::extension::error_screen)},
    {"has_completed_onboarding", flag<0>(&extension::has_completed_onboarding)},
    {"used_feature_diable_firefox_protection",
     flag<1>(&extension::used_feature_diable_firefox_protection)},
    {"used_feature_settings_page",
     flag<2>(&extension::used_feature_settings_page)},
    {"used_feature_page_action_revoke_exclude",
     flag<3>(&extension::used_feature_page_action_revoke_exclude)},
    {"used_feature_page_action_revoke_geopref",
     flag<4>(&extension::used_feature_page_action_revoke_geopref)},
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

void recordAllFlags() {
  auto settings = SettingsHolder::instance();
  const uint32_t currentBitFlags = settings->extensionTelemetryFlags();
  auto readFlag = [currentBitFlags](BooleanMetric* metric, uint8_t bit) {
    // Check if the specified bit is set
    bool isBitSet = (currentBitFlags & (1 << bit)) != 0;
    // Set the metric value based on the bit's state
    metric->set(isBitSet);
  };
  using namespace mozilla::glean;
  readFlag(&extension::has_completed_onboarding, 0);
  readFlag(&extension::used_feature_diable_firefox_protection, 1);
  readFlag(&extension::used_feature_settings_page, 2);
  readFlag(&extension::used_feature_page_action_revoke_exclude, 3);
  readFlag(&extension::used_feature_page_action_revoke_geopref, 4);
}

void startSession() {
  // Read the current bitflags to the ping before we submit it.
  recordAllFlags();
  mozilla::glean_pings::Extensionsession.submit("extension_start");
}
void stopSession() {
  mozilla::glean_pings::Extensionsession.submit("extension_stop");
}
}  // namespace WebextensionTelemetry
