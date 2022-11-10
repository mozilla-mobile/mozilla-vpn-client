/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QStringList>

class SettingsHolder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsHolder)

 public:
#define SETTING(type, toType, getter, setter, ...)                        \
  Q_PROPERTY(type getter READ getter WRITE setter NOTIFY getter##Changed) \
  Q_SIGNAL void getter##Changed();

#include "settingslist.h"
#undef SETTING

  Q_PROPERTY(QString placeholderUserDNS READ placeholderUserDNS CONSTANT)

  SettingsHolder();
  ~SettingsHolder();

  static SettingsHolder* instance();

  bool firstExecution() const { return m_firstExecution; }

  enum DnsProvider {
    Gateway = 0,
    BlockAll = 1,
    BlockAds = 2,
    BlockTracking = 3,
    Custom = 4,
  };
  Q_ENUM(DnsProvider)

  // These 3 methods can be used to store/restore/rollback settings.
  Q_INVOKABLE bool beginTransaction();
  Q_INVOKABLE bool commitTransaction();
  Q_INVOKABLE bool rollbackTransaction();

#ifdef UNIT_TEST
  bool inTransaction() const { return m_settingsJournal; }
  bool recoveredFromJournal() const { return m_recoverFromJournal; }
#endif

  // Don't use this directly!
  QVariant rawSetting(const QString& key) const;

#ifdef UNIT_TEST
  void setRawSetting(const QString& key, const QVariant& value);
  void doNotClearOnDTOR() { m_doNotClearOnDTOR = true; }
#endif

  QString getReport() const;

  void clear();

  void sync();

#define SETTING(type, toType, getter, setter, has, ...) \
  bool has() const;                                     \
  type getter() const;                                  \
  void setter(const type& value);

#include "settingslist.h"
#undef SETTING

  void removeEntryServer();

  // Delete _ALL_ the settings. Probably this method is not what you want to
  // use.
  void hardReset();

  QString settingsFileName() const;

  // Addon specific

  struct AddonSettingQuery {
    QString m_addonId;
    QString m_addonGroup;
    QString m_setting;
    QString m_defaultValue;

    AddonSettingQuery(const QString& ai, const QString& ag, const QString& s,
                      const QString& dv)
        : m_addonId(ai), m_addonGroup(ag), m_setting(s), m_defaultValue(dv) {}
  };
  QString getAddonSetting(const AddonSettingQuery& query);
  void setAddonSetting(const AddonSettingQuery& query, const QString& value);
  void clearAddonSettings(const QString& group);

 private:
  explicit SettingsHolder(QObject* parent);

  QString placeholderUserDNS() const;

  bool finalizeTransaction();

  QString journalSettingFileName() const;

  void maybeSaveInTransaction(const QString& key, const QVariant& oldValue,
                              const QVariant& newValue, const char* signalName,
                              bool userSettings);

  // Addon specific

  static QString getAddonSettingKey(const AddonSettingQuery& query);

 signals:
  void addonSettingsChanged();
  void inTransactionChanged();

 private:
  QSettings m_settings;

  bool m_firstExecution = false;

  QSettings* m_settingsJournal = nullptr;
  QMap<QString, QPair<const char*, QVariant>> m_transactionChanges;

#ifdef UNIT_TEST
  bool m_doNotClearOnDTOR = false;
  bool m_recoverFromJournal = false;
#endif
};

#endif  // SETTINGSHOLDER_H
