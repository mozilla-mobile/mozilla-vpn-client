/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDON_H
#define ADDON_H

#include <QJSValue>
#include <QObject>
#include <QQmlEngine>
#include <QTranslator>

#include "settingsholder.h"
#include "state/addonstate.h"

class AddonConditionWatcher;
class QJsonObject;

class AddonApi;
constexpr const char* ADDON_SETTINGS_GROUP = "addon";
constexpr const char* ADDON_DEFAULT_STATUS = "Unknown";
constexpr const char* ADDON_SETTINGS_STATUS_KEY = "state";

class Addon : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Addon)
  QML_NAMED_ELEMENT(MZAddon)
  QML_UNCREATABLE("")

  Q_PROPERTY(QString id READ id CONSTANT)
  Q_PROPERTY(QString name MEMBER m_name CONSTANT)
  Q_PROPERTY(AddonState* state READ state CONSTANT)
  Q_PROPERTY(Addon* parent MEMBER m_parent CONSTANT)

 public:
  enum Type {
    TypeGuide,
    TypeI18n,
    TypeMessage,
    TypeReplacer,
    TypeTutorial,
  };
  Q_ENUM(Type);

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
  const QString& manifestFileName() const { return m_manifestFileName; }

  virtual void setState(AddonState* state) { m_state = state; }
  AddonState* state() const {
    Q_ASSERT(m_state);
    return m_state;
  }

  virtual void retranslate();

  virtual bool enabled() const { return m_enabled; }

  AddonApi* api();

  virtual void enable();
  virtual void disable();

  Q_INVOKABLE virtual Addon* as(Type) { return nullptr; }

  void setParent(Addon* addon) { m_parent = addon; }

 signals:
  void conditionChanged(bool enabled);
  void dataChanged();

  void retranslationCompleted();

 protected:
  Addon(QObject* parent, const QString& manifestFileName, const QString& id,
        const QString& name);

 private:
  void updateAddonStatus(Status newStatus);

  bool evaluateJavascript(const QJsonObject& javascript);
  bool evaluateJavascriptInternal(const QString& javascript, QJSValue* value);

  struct StatusQuery final : public SettingsHolder::AddonSettingQuery {
    explicit StatusQuery(const QString& ai)
        : SettingsHolder::AddonSettingQuery(ai, QString(ADDON_SETTINGS_GROUP),
                                            QString(ADDON_SETTINGS_STATUS_KEY),
                                            QString(ADDON_DEFAULT_STATUS)) {}
  };

 private:
  const QString m_manifestFileName;
  const QString m_id;
  const QString m_name;

  QTranslator m_translator;

  AddonState* m_state = nullptr;

  Addon* m_parent = nullptr;
  AddonApi* m_api = nullptr;
  AddonConditionWatcher* m_conditionWatcher = nullptr;

  Status m_status = Unknown;

  QJSValue m_jsEnableFunction;
  QJSValue m_jsDisableFunction;

  bool m_enabled = false;
};

#endif  // ADDON_H
