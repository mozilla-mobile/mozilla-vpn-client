/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTFILTERING_H
#define ADJUSTFILTERING_H

#include <QMap>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>

#include "constants.h"

class AdjustFiltering final {
 public:
  QSet<QString> allowList{
      "adid",
      "android_uuid",
      "app_token",
      "att_status",
      "attribution_deeplink",
      "attribution_token",
      "bundle_id",
      "created_at",
      "device_type",
      "environment",
      "event_token",
      "google_play_instant",
      "gps_adid",
      "idfv",
      "needs_response_details",
      "os_name",
      "os_version",
      "package_name",
      "reference_tag",
      "referrer",
      "referrer_api",
      "sent_at",
      "source",
      "tracking_enabled",
      "zone_offset",
  };

  QMap<QString, QString> denyList{
      {"api_level", "29"},
      {"app_name", "default"},
      {"app_version", "2"},
      {"app_version_short", "2"},
      {"base_amount", "0"},
      {"connectivity_type", "0"},
      {"country", "US"},
      {"cpu_type", "arm64"},
      {"device_manufacturer", "xxxxx"},
      {"display_height", "0"},
      {"display_width", "0"},
      {"engagement_type", "0"},
      {"event_buffering_enabled", "0"},
      {"event_cost_id", "xxxxx"},
      {"gps_adid_attempt", "1"},
      {"gps_adid_lower_md5", "0"},
      {"gps_adid_lower_sha1", "0"},
      {"gps_adid_upper_md5", "0"},
      {"gps_adid_upper_sha1", "0"},
      {"hardware_name", "xxxx"},
      {"idfa", "xxxxx"},
      {"ios_uuid", "xxxxx"},
      {"language", "en"},
      {"last_interval", "0"},
      {"manufacturer", "default"},
      {"mcc", "0"},
      {"mnc", "0"},
      {"network_type", "0"},
      {"nonce", "0"},
      {"os_build", "xxxxx"},
      {"platform", "default"},
      {"primary_dedupe_token", "xxxxx"},
      {"random_user_id", "xxxxx"},
      {"region", "xxxxx"},
      {"screen_density", "0"},
      {"screen_format", "0"},
      {"screen_size", "0"},
      {"session_count", "1"},
      {"session_length", "0"},
      {"skadn_registered_at", "0"},
      {"started_at", "0"},
      {"store_name", "xxxxx"},
      {"subsession_count", "0"},
      {"terms_signed", "0"},
      {"time_spent", "0"},
      {"tracker_token", "xxxxx"},
      {"ui_mode", "0"},
  };

  struct MirrorParam {
    QString m_mirrorParamName;
    QString m_defaultValue;
  };

  QMap<QString, MirrorParam> mirrorList{
      {"device_name", {"device_type", Constants::PLATFORM_NAME}},
      {"installed_at", {"created_at", "0"}},
      {"updated_at", {"created_at", "0"}},
  };

  static AdjustFiltering* instance();

  // Public for unit-tests. Returns a new list of parameters filtering out what
  // we don't like.
  QUrlQuery filterParameters(QUrlQuery& parameters,
                             QStringList& unknownParameters);

  void allowField(const QString& field);
  void denyField(const QString& field, const QString& param);
  void mirrorField(const QString& field, const MirrorParam& param);
};

#endif  // ADJUSTFILTERING_H
