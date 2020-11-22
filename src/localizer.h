/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QAbstractListModel>
#include <QTranslator>

class SettingsHolder;

class Localizer final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Localizer)

  Q_PROPERTY(QString systemLanguage READ systemLanguage CONSTANT)
  Q_PROPERTY(
      QString systemLocalizedLanguage READ systemLocalizedLanguage CONSTANT)

 public:
  enum ServerCountryRoles {
    LanguageRole = Qt::UserRole + 1,
    LocalizedLanguageRole,
    CodeRole,
  };

  static Localizer* instance();

  Localizer();
  ~Localizer();

  Q_INVOKABLE QString localizeSubscriptionCurrencyValue() const;

  void initialize(const QString& code);

  void loadLanguage(const QString& code);

  QString systemLanguage() const { return languageName(""); }

  QString systemLocalizedLanguage() const { return localizedLanguageName(""); }

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 private:
  QString languageName(const QString& code) const;
  QString localizedLanguageName(const QString& code) const;

  bool loadLanguageInternal(const QString& code);

 private:
  QTranslator m_translator;

  QString m_code;
  QStringList m_languages;
};

#endif  // LOCALIZER_H
