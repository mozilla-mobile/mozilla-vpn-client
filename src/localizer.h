/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QAbstractListModel>
#include <QLocale>
#include <QTranslator>

class Collator;
class SettingsHolder;

class Localizer final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Localizer)

  Q_PROPERTY(QString code READ code WRITE setCode NOTIFY codeChanged)
  Q_PROPERTY(QString previousCode READ previousCode NOTIFY previousCodeChanged)
  Q_PROPERTY(bool hasLanguages READ hasLanguages CONSTANT)
  Q_PROPERTY(QLocale locale READ locale NOTIFY localeChanged)
  Q_PROPERTY(bool isRightToLeft READ isRightToLeft NOTIFY codeChanged)

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

  static QString systemLanguageCode();

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

  Q_INVOKABLE QString localizedCityName(const QString& code,
                                        const QString& city);

  Q_INVOKABLE QString localizeCurrency(double value,
                                       const QString& currencyIso4217);

  // Returns the major part of the string in case the language code is in the
  // format 'en-FO' or 'en_FO'
  static QString majorLanguageCode(const QString& code);

  QLocale locale() const { return m_locale; }

  bool isRightToLeft() const;

 signals:
  void codeChanged();
  void previousCodeChanged();
  void localeChanged();

 private:
  static QString languageName(const QString& code);
  static QString localizedLanguageName(const QString& code);
  static bool languageSort(const Language& a, const Language& b,
                           Collator* collator);

  bool loadLanguageInternal(const QString& code);

  // This method is not used. It exists just to add the installer strings into
  // the QT language files.
  static void macOSInstallerStrings();

  static QString retrieveCurrencySymbolFallback(const QString& currencyIso4217,
                                                const QLocale& currentLocale);

 private:
  QTranslator m_translator;

  QString m_code;

  QLocale m_locale;

  QList<Language> m_languages;
};

#endif  // LOCALIZER_H
