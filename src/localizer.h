/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QAbstractListModel>
#include <QTranslator>

class QCollator;
class SettingsHolder;

class Localizer final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Localizer)

  Q_PROPERTY(QString code READ code WRITE setCode NOTIFY codeChanged)
  Q_PROPERTY(QString previousCode READ previousCode NOTIFY previousCodeChanged)
  Q_PROPERTY(bool hasLanguages READ hasLanguages CONSTANT)

  struct Language {
    QString m_code;
    QString m_name;
    QString m_localizedName;
  };

 public:
  enum ServerCountryRoles {
    LanguageRole = Qt::UserRole + 1,
    LocalizedLanguageRole,
    CodeRole,
  };

  static Localizer* instance();

  Localizer();
  ~Localizer();

  void initialize();

  void loadLanguage(const QString& code);

  bool hasLanguages() const { return m_languages.length() > 1; }

  const QString& code() const { return m_code; }
  void setCode(const QString& code) { loadLanguage(code); }

  QString previousCode() const;

  QStringList languages() const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void codeChanged();
  void previousCodeChanged();

 private:
  static QString languageName(const QString& code);
  static QString localizedLanguageName(const QString& code);
  static bool languageSort(const Language& a, const Language& b,
                           QCollator* collator);

  bool loadLanguageInternal(const QString& code);

 private:
  QTranslator m_translator;

  QString m_code;

  QList<Language> m_languages;
};

#endif  // LOCALIZER_H
