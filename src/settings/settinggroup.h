/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GROUPEDSETTINGS_H
#define GROUPEDSETTINGS_H

#include <QObject>
#include <QVariant>

#include "settingsconnector.h"

/**
 * @brief Represents a group of dynamic settings. Dynamic setttings can be
 * defined at runtime.
 *
 * This abstraction serves as an aggregator of underlying storage settings.
 * These settings necessarily need to be grouped under a QSettings group i.e.
 * their keys need to share the same prefix followed by a slash ("/").
 *
 * Because settings under a SettingsGroup can be dynamically generated, upon
 * initialization of a new SettingsGroup object the underlying storage is probed
 * for any stored setting with the group prefix and those settings are
 * registered to the SettingsManager.
 *
 * A SettingsGroups is useful when:
 *
 * * Defining metrics dynamically at runtime is required or
 * * Reacting to changes for _any_ setting under the group is necessary or
 * * It makes sense to have metrics grouped for code semantics.
 *
 * It is allowed to create SettingGroups that are subsets or supersets of other
 * SettingGroups. However, this can be tricky if the different groups are
 * configured differently e.g. one has removeWhenReset set to true, while the
 * other has removeWhenReset set to false. Whichever group register the
 * underlying setting first will have their configurations enforced. There are
 * Q_ASSERTs set up in debug mode to catch such situations.
 *
 */
class SettingGroup : public QObject {
  Q_OBJECT

 public:
  SettingGroup(QObject* parent, SettingsConnector* settingsConnector,
               const QString& groupKey, bool removeWhenReset = true,
               bool sensitiveSetting = false,
               const QStringList& acceptedKeys = QStringList());
  ~SettingGroup();

  /**
   * @brief Get the stored value for a setting in this group. If not setting, a
   * null QVariant is returned.
   *
   * @return QVariant
   */
  Q_INVOKABLE QVariant get(const QString& key) const;

  /**
   * @brief Set the value of a setting in this group.
   *
   * If the value is the same as the stored value, this is a no-op.
   * If the metric was never set and the provided value is the default value,
   * it will be set.
   *
   * If the setting is not in storage yet, it will
   * be added to the group.
   *
   * @param value
   */
  void set(const QString& key, QVariant value);

  /**
   * @brief Removes the stored values for all settings in this group.
   *
   */
  void remove();

 signals:
  /**
   * @brief This signal is emmited whenever any of the settings under this group
   * has their underlying storage value changed.
   *
   */
  void changed();

 private:
  void addSetting(const QString& key);
  bool isAcceptedKey(const QString& key);
  QString buildSettingKeyWithGroupPrefix(const QString& key) const {
    return QString("%1/%2").arg(m_groupKey).arg(key);
  };

 private:
  QString m_groupKey;

  bool m_sensitiveSetting = false;
  bool m_removeWhenReset = true;
  QStringList m_acceptedKeys;

  SettingsConnector* m_settingsConnector;
};

#endif  // GROUPEDSETTINGS_H
