/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QByteArrayView>
#include <QString>

namespace SentrySniffer {
enum ContentType { Unknown, Ping, CrashReport };

struct SniffResult {
  ContentType type;
  std::optional<QString> id;
};

/**
 * @brief Reads a Sentry envelope and checks if it contains crash data.
 *
 * @param data - The raw sentry data.
 * @return ContentType -> Returns a content type.
 */
auto parseEnvelope(const QByteArrayView& data) -> SniffResult;
}  // namespace SentrySniffer
