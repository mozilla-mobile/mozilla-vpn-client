/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTING_H
#define SETTING_H

#include <QObject>
#include <QVariant>

/**
 * @brief Represents a setting stored in the underlying QSettings storage.
 *
 * FAQ: Why is this not using a template type for the underlying setting type?
 * A: Because we can't apply the Q_OBJECT macro to a template class.
 */
class Setting : public QObject {
  Q_OBJECT

 public:
  ~Setting();

  QString key() const { return m_key; }

  /**
   * @brief Get the stored value for this setting. If not set, will return the
   * default value.
   *
   * @return T
   */
  QVariant get() const;

  /**
   * @brief Set a value for this setting.
   *
   * If the value is the same as the stored value, this is a no-op.
   * If the metric was never set and the provided value is the default value,
   * it will be set.
   *
   * @param value
   */
  void set(QVariant value) const;

  /**
   * @brief Resets the value for this setting if removeWhenReset is true.
   * Otherwise, this is no-op.
   *
   */
  void reset() const;

  /**
   * @brief Removes the stored value for this setting.
   *
   */
  void remove() const;

  /**
   * @brief Checks whether or not this setting has a stored value.
   *
   */
  bool isSet() const;

  /**
   * @brief Gets a string formatted for adding this setting to logs.
   *
   * The setting value will be redacted in case this is a sensitive setting.
   *
   * The log line is formatted like so:
   * - key -> value
   * - key -> <Sensitive>
   *
   * @return QString
   */
  QString log() const;

 signals:
  /**
   * @brief This signal is emmited whenever the underlying storage value related
   * to the setting is changed.
   *
   * Check the documentation of the updater functions above to understand when
   * the underlying value is changed or not.
   *
   */
  void changed() const;

 private:
  Setting(QObject* parent, const QString& key,
          std::function<QVariant()> defaultValue, bool removeWhenReset,
          bool sensitiveSetting);

 private:
  QString m_key;

  std::function<QVariant()> m_defaultValue;

  bool m_sensitiveSetting;
  bool m_removeWhenReset;

  friend class SettingFactory;
  friend class SettingGroup;

#ifdef UNIT_TEST
  friend class TestSettingFactory;
  friend class TestSettingGroup;
#endif
};

#endif  // SETTING_H
