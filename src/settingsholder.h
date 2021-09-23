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

  enum DnsProvider {
    Gateway = 0,
    BlockAll = 1,
    BlockAds = 2,
    BlockTracking = 3,
    Custom = 4,
  };
  Q_ENUM(DnsProvider)

  QString getReport();

  void clear();

#define SETTING(type, toType, getter, setter, has, ...) \
  bool has() const;                                     \
  type getter() const;                                  \
  void setter(const type& value);

#include "settingslist.h"
#undef SETTING

  void removeEntryServer();

  QString envOrDefault(const QString& name, const QString& defaultValue) const;

 signals:

 private:
  explicit SettingsHolder(QObject* parent);

  QString placeholderUserDNS() const;

 private:
  QSettings m_settings;
};

#endif  // SETTINGSHOLDER_H
