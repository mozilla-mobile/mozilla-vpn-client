/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef settingsmanager_H
#define settingsmanager_H

#include <QSettings>

#include "loghandler.h"
#include "qsettingsconnector.h"
#include "setting.h"
#include "settinggroup.h"

/**
 * @brief The SettingsManager is a singleton class that manages the underlying
 * QtSetting storage object and serves the as the holder of all settings in the
 * codebase dynamic or static. The lifetime of this singleton is tied to the
 * lifetime of the QApplication.
 *
 * There are two types of settings currently in this codebase:
 *
 * * Static settings are settings defined at build time in a declarative manner
 * e.g. the SETTING macro or the EXPERIMENTAL_FEATURE macro.
 * * Dynamic settings are settings which are created on demand at runtime.
 *
 * All settings are created using SettingsManager::createOrGetSetting, which
 * creates a Setting object and registers it in the SettingsManager class.
 *
 * The only public constructor for Setting objects is
 * SettingsManager::createOrGetSetting, therefore everytime a
 * Setting object is created it is guaranteed to be registered in the settings
 * base.
 *
 */
class SettingsManager final : public QObject, public LogSerializer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsManager)

 public:
  ~SettingsManager();

  /**
   * @brief Get the SettingsManager singleton.
   *
   * Creates it if it has not been created yet.
   * The lifetime o the singleton is tied to the lifetime of the QApplication.
   *
   * @return SettingsManager*
   */
  static SettingsManager* instance();

  // LogSerializer interface
  void serializeLogs(
      std::function<void(const QString& name, const QString& logs)>&& callback)
      override;

  /**
   * @brief Returns a registered setting based on their key.
   *
   * This will return a nullptr in case the setting is not registered.
   *
   * @param key
   * @return Setting*
   */
  Setting* getSetting(const QString& key) {
    return m_registeredSettings.value(key);
  }

  QString settingsFileName();

  /**
   * @brief Soft reset all registered settings.
   *
   * If the setting is configured to be removed on reset this will reset it,
   * otherwise it will stay there.
   *
   */
  void reset();

  /**
   * @brief Hard reset all registered settings.
   *
   * All settings are cleared, regardless of configuration.
   *
   */
  void hardReset();

  /**
   * @brief Writes any unsaved changes to permanent storage,
   * and reloads any settings that have been changed in the
   * meantime by another application.
   *
   * This function is called automatically from QSettings's
   * destructor and by the event loop at regular intervals,
   * so you normally don't need to call it yourself.
   *
   * Description copied from: https://doc.qt.io/qt-6/qsettings.html#sync
   *
   */
  void sync() { instance()->m_settings.sync(); };

  /**
   * @brief Construct a new Setting object and register it.
   *
   * If another setting with the same key is already registered, no new setting
   * is registered and a pointer to the existing setting is returned. This
   * function will crash in debug mode in case the existing setting has a
   * different configuration from the new setting.
   *
   * @param key A QSettings valid key.
   * @param defaultValue The default value to be returned when this setting is
   * not set. Default value will be null if unset.
   * @param removeWhenReset Whether or not this setting should actually be
   * removed when `reset` is called. Default is true.
   * @param sensitiveSetting Whether or not this is a sensitive setting i.e. a
   * setting that must not be logged in plain text. Default is false.
   */
  static Setting* createOrGetSetting(
      const QString& key,
      std::function<QVariant()> defaultValue = []() { return QVariant(); },
      bool removeWhenReset = true, bool sensitiveSetting = false);

  /**
   * @brief Construct a new SettingGroup object
   *
   * @param groupKey The group key will be used as a prefix for the key of all
   * settings added to the group e.g. groupKey/mySetting.
   * @param removeWhenReset Whether or not this settings in this group should
   * actually be removed when `reset` is called.
   * @param sensitiveSetting Whether or not settings in this group are
   * sensitive settings i.e. settings that must not be logged in plain text.
   * @param acceptedKeys When this list has at least one member, only the listed
   * keys will be allowed for storing under this group. Other keys will be
   * ignored.
   */
  static SettingGroup createSettingGroup(
      const QString& groupKey, bool removeWhenReset = true,
      bool sensitiveSetting = false, QStringList acceptedKeys = QStringList());

#ifdef UNIT_TEST
  /**
   * @brief Destroys the SettingsManager singleton and clear stores.
   *
   * The SettingsManager Singleton exists for the entire lifetime of the
   * application. The way our tests are set up now, we have most test cases
   * running under the same application instance. This way SettingsManager
   * state can leak amongst tests and therefore this function is required.
   *
   */
  static void testCleanup();
#endif

 private:
  SettingsManager(QObject* parent);

  void registerSetting(Setting* setting);

 private:
  QMap<QString, Setting*> m_registeredSettings;
  QSettings m_settings;
  QSettingsConnector m_settingsConnector;

#ifdef UNIT_TEST
  friend class TestSettingsManager;
  friend class TestSettings;
#endif
};

#endif  // settingsmanager_H
