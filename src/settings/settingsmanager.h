/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef settingsmanager_H
#define settingsmanager_H

#include <QSettings>

#include "loghandler.h"
#include "setting.h"
#include "settinggroup.h"

/**
 * @brief The SettingsManager is a singleton class that manages the underlying
 * QtSetting storage object and serves the as the holder of all settings in the
 * codebase dynamic or static. The lifetime of this singleton is tied to the
 * lifetime of the QApplication.
 *
 * There are two types of settings currently in this codebase.
 *
 * * Static settings are settings defined at build time in a declarative manner
 * e.g. the SETTING macro or the EXPERIMENTAL_FEATURE macro.
 * * Dynamic settings are settings which are created on demand at runtime.
 *
 * All settings are created using the SettingsFactory, which creates a Setting
 * object and registers it in the SettingsManager class.
 *
 * The only public constructor for Setting objects is the SettingsFactory
 * constructor, therefore everytime a Setting object is created it is guaranteed
 * to be registered in the settings base.
 *
 */
class SettingsManager final : public QObject, public LogSerializer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsManager)

 public:
  ~SettingsManager();

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
  static Setting* getSetting(const QString& key) {
    return instance()->m_registeredSettings.value(key);
  }

  static QString settingsFileName();

  /**
   * @brief Soft reset all registered settings.
   *
   * If the setting is configured to be removed on reset this will reset it,
   * otherwise it will stay there.
   *
   */
  static void reset();

  /**
   * @brief Hard reset all registered settings.
   *
   * All settings are cleared, regardless of configuration.
   *
   */
  static void hardReset();

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
  static void sync() { instance()->m_settings.sync(); };

#ifdef UNIT_TEST
  /**
   * @brief Destroys the SettingsManager singleton and clear stores.
   *
   * The SettingsManager Singleton exists for the entire lifetime of the
   * application. The way our tests are set up now, we have most test cases
   * running under the same application instance. This way SettingsManager state
   * can leak amongst tests and therefore this function is required.
   *
   */
  static void testCleanup();
#endif

 private:
  SettingsManager(QObject* parent);
  static SettingsManager* instance();

  static void registerSetting(Setting* setting);

 private:
  QMap<QString, Setting*> m_registeredSettings;
  QSettings m_settings;

  friend class Setting;
  friend class SettingGroup;
  friend class SettingFactory;

#ifdef UNIT_TEST
  friend class TestSettingsManager;
  friend class TestSettings;
#endif
};

#endif  // settingsmanager_H
