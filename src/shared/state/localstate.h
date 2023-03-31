/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONLOCALSTATE_H
#define ADDONLOCALSTATE_H

#include "settingsholder.h"
#include "statebase.h"

/**
 * @brief Local State implementation,
 * in which state does persist throughout restarts.
 *
 * The storage backend for the LocalState is the SettingsHolder.
 *
 * The QSettings object in SettingsHolder can be directly accessed by this
 * class, because it is a friend class of the SettingsHolder class.
 */
class LocalState final : public StateBase {
 public:
  ~LocalState();

  /**
   * @brief Construct an LocalState object from the JSON object under the
   * `state` key of an Addon's manifest.
   *
   * The manifest should be in the following format:
   *
   * ```
   * {
   *    [key: string]: {
   *      "type": "boolean" | "string" | "number",
   *      "default": boolean | string | number
   *    }
   * }
   * ```
   *
   * Invalid properties on the manifest will be ignored. In the worst case
   * if the manifest is completely wrong an empty state will be generated.
   *
   * @param storageIdentifier The key under which this state will be stored in
   * the local storage backend. This should be unique. A warning will be loggerd
   * when another LocalState has already been created with the same identifier
   * during a given application run. For addons, prefer using addon id's.
   * @param manifest The addon's manifest.
   * @return State the generated state.
   */
  LocalState(const QString& storageIdentifier, const QJsonObject& manifest);

 private:
  QJsonValue getInternal(const QString& key) const override;
  void setInternal(const QString& key, const QJsonValue& value) override;
  void clearInternal(const QString& key = "") override;

  /**
   * @brief Gets the settings from the settings holder. This class is a friend
   * of that class, so it can access the internal QSettings directly.
   *
   * @return QSettings The application's internal settings.
   */
  static QSettings* settings() {
    return &SettingsHolder::instance()->m_settings;
  }

  static QString getStorageIdentifierForKey(const QString& storageIdentifier,
                                            const QString& key);

  const QString m_storageIdentifier;
};

#endif  // ADDONLOCALSTATE_H
