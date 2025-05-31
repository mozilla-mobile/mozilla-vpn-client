/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QAbstractListModel>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QTranslator>

class QTranslator;
class SettingsHolder;

class Localizer final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Localizer)

  Q_PROPERTY(bool hasLanguages READ hasLanguages CONSTANT)
  Q_PROPERTY(QLocale locale READ locale NOTIFY localeChanged)
  Q_PROPERTY(bool isRightToLeft READ isRightToLeft NOTIFY localeChanged)

  struct Language {
    QString m_code;
    QString m_languageCode;
    QString m_countryCode;
    QString m_nativeLanguageName;
    bool m_rtl;
  };

 public:
  enum LocalizerRoles {
    LocalizedLanguageNameRole = Qt::UserRole + 1,
    NativeLanguageNameRole,
    CodeRole,
    RTLRole,
  };

  static Localizer* instance();

  Localizer();
  ~Localizer();

  void initialize();

  bool hasLanguages() const { return !m_languages.isEmpty(); }

  // This returns the language code from the settings, and, if it's null, it
  // returns the system language code.
  Q_INVOKABLE QString languageCodeOrSystem() const;

  QStringList languages() const;

  Q_INVOKABLE QString localizeCurrency(double value,
                                       const QString& currencyIso4217);

  // Returns the major part of the string in case the language code is in the
  // format 'en-FO' or 'en_FO'
  static QString majorLanguageCode(const QString& code);

  QLocale locale() const { return m_locale; }

  bool isRightToLeft() const;

  static QList<QPair<QString, QString>> parseBCP47Languages(
      const QStringList& languages);
  static QList<QPair<QString, QString>> parseIOSLanguages(
      const QStringList& languages);

  static void forceRTL();

  // Public for testing
  static QMap<QString, double> loadTranslationCompleteness(
      const QString& fileName);

  static QString getTranslatedCountryName(const QString& countryCode,
                                          const QString& countryName);
  static QString getTranslatedCityName(const QString& cityName);

  Q_INVOKABLE QString formatDate(const QDateTime& nowDateTime,
                                 const QDateTime& messageDateTime,
                                 const QString& yesterday);

  QStringList fallbackForLanguage(const QString& code) const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void localeChanged();

 private:
  QString localizedLanguageName(const QString& languageCode) const;
  static QString nativeLanguageName(const QLocale& locale, const QString& code);

  QString systemLanguageCode() const;

  void loadLanguagesFromI18n();
  bool loadLanguage(const QString& requestedLocaleCode);
  QString findLanguageCode(const QString& languageCode,
                           const QString& countryCode) const;

  void settingsChanged();

  bool createTranslator(const QLocale& locale);

  void maybeLoadLanguageFallback(const QString& code);

  void maybeLoadLanguageFallbackData();

  static QString getTranslationCode();

  static QString getCapitalizedStringFromI18n(const QString& id);

 private:
  QList<QTranslator*> m_translators;

  QString m_code;

  QLocale m_locale;

  QList<Language> m_languages;
  QMap<QString, double> m_translationCompleteness;
  QMap<QString, QStringList> m_translationFallback;

#ifdef UNIT_TEST
  friend class TestLocalizer;
#endif
};

// Custom translator class that also handles server localization.
class VpnTranslator final : public QTranslator {
  Q_OBJECT
  friend class Localizer;

 protected:
  VpnTranslator(QObject* parent = nullptr) : QTranslator(parent){};

 public:
  QString translate(const char* context, const char* sourceText,
                    const char* disambiguation = nullptr,
                    int n = -1) const override;
};

#endif  // LOCALIZER_H
