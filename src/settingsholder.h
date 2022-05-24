/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QDateTime>
#include <QStringList>
#include <QObject>
#include <QSettings>

class SettingsHolder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsHolder)

 public:
#define SETTING(type, toType, getter, setter, ...)                        \
  Q_PROPERTY(type getter READ getter WRITE setter NOTIFY getter##Changed) \
  Q_SIGNAL void getter##Changed(const type& value);

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

  // Don't use this directly!
  QVariant rawSetting(const QString& key) const;

#ifdef UNIT_TEST
  void setRawSetting(const QString& key, const QVariant& value);
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

  QString envOrDefault(const QString& name, const QString& defaultValue) const;

  // Delete _ALL_ the settings. Probably this method is not what you want to
  // use.
  void hardReset();

  QString settingsFileName() const;

 private:
  explicit SettingsHolder(QObject* parent);

  QString placeholderUserDNS() const;

 private:
  QSettings m_settings;
  bool m_firstExecution = false;
};

#endif  // SETTINGSHOLDER_H
