/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONINDEX_H
#define ADDONINDEX_H

#include "addondirectory.h"

#include <QObject>
#include <QString>

class QByteArray;
class QJsonObject;
class QJsonValue;
class Addon;

constexpr const char* ADDONS_API_VERSION = "0.1";
constexpr const char* ADDON_INDEX_FILENAME = "manifest.json";
constexpr const char* ADDON_INDEX_SIGNATURE_FILENAME = "manifest.json.sig";

// This struct can be partially empty in case the sha does not match, or the
// addon does not need to be loaded for unmatched conditions.
struct AddonData {
  QByteArray m_sha256;
  QString m_addonId;
  Addon* m_addon;
};

class AddonIndex final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonIndex);

 public:
  explicit AddonIndex(AddonDirectory* dir);

  bool getOnDiskAddonsList(QList<AddonData>* addonsList);
  void update(const QByteArray& index, const QByteArray& indexSignature);

 signals:
  void indexUpdated(const QList<AddonData>& addons);

 private:
  bool read(QByteArray& index, QByteArray& indexSignature);
  bool write(const QByteArray& index, const QByteArray& indexSignature);
  bool validate(const QByteArray& index, const QByteArray& indexSignature,
                QJsonObject* indexObj);

  // Helpers
  static bool validateIndex(const QByteArray& index, QJsonObject* indexObj);
  static bool validateSingleAddonIndex(const QJsonValue& addonValue);
  static bool validateIndexSignature(const QByteArray& index,
                                     const QByteArray& indexSignature);

  static QList<AddonData> extractAddonsFromIndex(const QJsonObject& indexObj);

 private:
  AddonDirectory* m_addonDirectory = nullptr;
};

#endif  // ADDONINDEX_H
