/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDON_H
#define ADDON_H

#include <QJSValue>
#include <QObject>
#include <QTranslator>
#include "settingsholder.h"

class AddonConditionWatcher;
class QJsonObject;

class AddonApi;
constexpr const char* ADDON_SETTINGS_GROUP = "addon";
constexpr const char* ADDON_DEFAULT_STATE = "Unknown";
constexpr const char* ADDON_SETTINGS_STATE_KEY = "state";

class Addon : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Addon)

  Q_PROPERTY(QString id READ id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)

 public:
  enum State {
    // Initial state. This should be used only during the loading.
    Unknown,

    // The add-on has just been installed. This is the first time the device
    // sees this add-on. The add-on is not enabled yet.
    Installed,

    // The add-on is enabled.
    Enabled,

    // The add-on is disabled.
    Disabled,
  };
  Q_ENUM(State);

  static Addon* create(QObject* parent, const QString& manifestFileName);

  static bool evaluateConditions(const QJsonObject& conditions);

  static AddonConditionWatcher* maybeCreateConditionWatchers(
      Addon* addon, const QJsonObject& conditions);

  virtual ~Addon();

  const QString& id() const { return m_id; }
  const QString& type() const { return m_type; }
  const QString& manifestFileName() const { return m_manifestFileName; }

  virtual void retranslate();

  virtual bool enabled() const { return m_enabled; }

  AddonApi* api();

  virtual void enable();
  virtual void disable();

 signals:
  void conditionChanged(bool enabled);
  void retranslationCompleted();

 protected:
  Addon(QObject* parent, const QString& manifestFileName, const QString& id,
        const QString& name, const QString& type);

 private:
  void updateAddonState(State newState);

  bool evaluateJavascript(const QJsonObject& javascript);
  bool evaluateJavascriptInternal(const QString& javascript, QJSValue* value);

  struct StateQuery final : public SettingsHolder::AddonSettingQuery {
    explicit StateQuery(const QString& ai)
        : SettingsHolder::AddonSettingQuery(ai, QString(ADDON_SETTINGS_GROUP),
                                            QString(ADDON_SETTINGS_STATE_KEY),
                                            QString(ADDON_DEFAULT_STATE)) {}
  };

 private:
  const QString m_manifestFileName;
  const QString m_id;
  const QString m_name;
  const QString m_type;

  QTranslator m_translator;

  AddonApi* m_api = nullptr;
  AddonConditionWatcher* m_conditionWatcher = nullptr;

  State m_state = Unknown;

  QJSValue m_jsEnableFunction;
  QJSValue m_jsDisableFunction;

  bool m_enabled = false;
};

#endif  // ADDON_H
