/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localizer.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocale>

namespace {
Logger logger(LOG_MAIN, "Localizer");
Localizer* s_instance = nullptr;

struct StaticLanguage {
  QString m_name;
  QString m_localizedName;
};

// Some languages do not have the right localized/non-localized names in the QT
// framework (and some are missing entirely). This static map is the fallback
// when this happens.
QMap<QString, StaticLanguage> s_languageMap{
    {"co", StaticLanguage{"Corsu", ""}}};
}  // namespace

// static
Localizer* Localizer::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

Localizer::Localizer() {
  MVPN_COUNT_CTOR(Localizer);

  Q_ASSERT(!s_instance);
  s_instance = this;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->hasLanguageCode()) {
    m_code = settingsHolder->languageCode();
  }

  if (m_code.isEmpty()) {
    QLocale locale = QLocale::system();
    m_code = locale.bcp47Name();
  }

  initialize();
}

Localizer::~Localizer() {
  MVPN_COUNT_DTOR(Localizer);

  Q_ASSERT(s_instance = this);
  s_instance = nullptr;
}

void Localizer::initialize() {
  loadLanguage(m_code);

  QCoreApplication::installTranslator(&m_translator);
  QDir dir(":/i18n");
  QStringList files = dir.entryList();
  for (const QString& file : files) {
    if (!file.startsWith("mozillavpn_") || !file.endsWith(".qm")) {
      continue;
    }

    QStringList parts = file.split(".");
    Q_ASSERT(parts.length() == 2);

    QString code = parts[0].remove(0, 11);
    m_languages.append(code);
  }
}

void Localizer::loadLanguage(const QString& code) {
  logger.log() << "Loading language:" << code;
  if (loadLanguageInternal(code)) {
    return;
  }

  logger.log() << "Loading default language (fallback)";
  loadLanguageInternal("en");
}

bool Localizer::loadLanguageInternal(const QString& code) {
  QLocale locale = QLocale(code);
  if (code.isEmpty()) {
    locale = QLocale::system();
  }

  QLocale::setDefault(locale);

  if (!m_translator.load(locale, "mozillavpn", "_", ":/i18n")) {
    logger.log() << "Loading the locale failed."
                 << "code";
    return false;
  }

  SettingsHolder::instance()->setLanguageCode(code);

  m_code = code;
  emit codeChanged();

  return true;
}

QString Localizer::languageName(const QString& code) const {
  if (s_languageMap.contains(code)) {
    QString languageName = s_languageMap[code].m_name;
    if (!languageName.isEmpty()) {
      return languageName;
    }
  }

  QLocale locale(code);
  if (code.isEmpty()) {
    locale = QLocale::system();
  }

  if (locale.language() == QLocale::C) {
    return "English (US)";
  }

  QString name = QLocale::languageToString(locale.language());

  // Capitalize the string.
  name.replace(0, 1, locale.toUpper(QString(name[0])));
  return name;
}

QString Localizer::localizedLanguageName(const QString& code) const {
  if (s_languageMap.contains(code)) {
    QString languageName = s_languageMap[code].m_localizedName;
    if (!languageName.isEmpty()) {
      return languageName;
    }
  }

  QLocale locale(code);
  if (code.isEmpty()) {
    locale = QLocale::system();
  }

  if (locale.language() == QLocale::C) {
    return "English (US)";
  }

  QString name = locale.nativeLanguageName();
  if (name.isEmpty()) {
    return languageName(code);
  }

  // Capitalize the string.
  name.replace(0, 1, locale.toUpper(QString(name[0])));
  return name;
}

QHash<int, QByteArray> Localizer::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[LanguageRole] = "language";
  roles[LocalizedLanguageRole] = "localizedLanguage";
  roles[CodeRole] = "code";
  return roles;
}

int Localizer::rowCount(const QModelIndex&) const {
  return m_languages.count();
}

QVariant Localizer::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case LanguageRole:
      return QVariant(languageName(m_languages.at(index.row())));

    case LocalizedLanguageRole:
      return QVariant(localizedLanguageName(m_languages.at(index.row())));

    case CodeRole:
      return QVariant(m_languages.at(index.row()));

    default:
      return QVariant();
  }
}
