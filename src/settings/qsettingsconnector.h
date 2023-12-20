/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QSETTINGSCONNECTOR_H
#define QSETTINGSCONNECTOR_H

#include <QObject>
#include <QSettings>

/**
 * @brief Exposes APIs to interact with an instance of QSettings.
 *
 */
class QSettingsConnector : public QObject {
 public:
  QSettingsConnector(QObject* parent, QSettings* settings);
  ~QSettingsConnector();

  /**
   * @brief Gets a value from the underlying storage.
   *
   */
  QVariant getValue(const QString& key) const;

  /**
   * @brief Sets a value in the underlying storage.
   *
   */
  void setValue(const QString& key, QVariant value) const;

  /**
   * @brief Removes values from the underlying storage.
   *
   * If group is provided, the removal will be in the context of the group.
   * If key is empty and group is provided, all keys under the group are
   * removed. If both are empty, the whole storage is cleared.
   *
   * @param key
   * @param group
   * @return QVariant
   */
  void remove(const QString& key, const QString& group = "") const;

  /**
   * @brief Gets all keys in storage.
   *
   * If group is provided, scopes the keys to the group.
   *
   * @param group
   * @return QStringList
   */
  QStringList getAllKeys(const QString& group = "") const;

  /**
   * @brief Checks if a given key has a value stored under it.
   */
  bool contains(const QString& key) const;

 private:
  QSettings* m_settings;
};

#endif  // QSETTINGSCONNECTOR_H
