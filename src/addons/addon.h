/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDON_H
#define ADDON_H

#include <QJSValue>
#include <QObject>
#include <QTranslator>

#include "settings/settinggroup.h"
#include "settings/settingsholder.h"
#include "state/addonstate.h"

class AddonConditionWatcher;
class QJsonObject;

class AddonApi;
class SettingGroup;

// A settings group that will contain all settings related to individual addons.
constexpr const char* ADDON_SETTINGS_GROUP = "addon";
constexpr const char* ADDON_SETTINGS_STATUS_KEY = "state";

class Addon : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Addon)

  Q_PROPERTY(QString id READ id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(AddonState* state READ state CONSTANT)

 public:
  enum Status {
    // Initial status. This should be used only during the loading.
    Unknown,

    // The add-on has just been installed. This is the first time the device
    // sees this add-on. The add-on is not enabled yet.
    Installed,

    // The add-on is enabled.
    Enabled,

    // The add-on is disabled.
    Disabled,
  };
  Q_ENUM(Status);

  static Addon* create(QObject* parent, const QString& manifestFileName);

  static bool evaluateConditions(const QJsonObject& conditions);

  static AddonConditionWatcher* maybeCreateConditionWatchers(
      Addon* addon, const QJsonObject& conditions);

  virtual ~Addon();

  const QString& id() const { return m_id; }
  const QString& type() const { return m_type; }
  const QString& manifestFileName() const { return m_manifestFileName; }
  AddonState* state() const {
    Q_ASSERT(m_state);
    return m_state;
  }

  virtual void retranslate();

  virtual bool enabled() const { return m_enabled; }

  AddonApi* api();

  virtual void enable();
  virtual void disable();

  const QMap<QString, double>& translationCompleteness() const {
    return m_translationCompleteness;
  }

 signals:
  void conditionChanged(bool enabled);
  void retranslationCompleted();
  void aboutToDisable();

 protected:
  Addon(QObject* parent, const QString& manifestFileName, const QString& id,
        const QString& name, const QString& type);

 private:
  void updateAddonStatus(Status newStatus);

  bool evaluateJavascript(const QJsonObject& javascript);
  bool evaluateJavascriptInternal(const QString& javascript, QJSValue* value);

  void unloadTranslators();
  bool createTranslator(const QLocale& locale);
  void maybeLoadLanguageFallback(const QString& code);

 private:
  const QString m_manifestFileName;
  const QString m_id;
  const QString m_name;
  const QString m_type;

  QList<QTranslator*> m_translators;
  QMap<QString, double> m_translationCompleteness;

  AddonState* m_state = nullptr;

  AddonApi* m_api = nullptr;
  AddonConditionWatcher* m_conditionWatcher = nullptr;

  Status m_status = Unknown;

  QJSValue m_jsEnableFunction;
  QJSValue m_jsDisableFunction;

  // Group of dynamic settings related to this specific addon.
  SettingGroup* m_settingGroup;

  bool m_enabled = false;
};

#endif  // ADDON_H
