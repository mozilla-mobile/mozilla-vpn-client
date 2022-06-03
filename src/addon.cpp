/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addon.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QCoreApplication>

namespace {
Logger logger(LOG_MAIN, "Addon");
}

Addon::Addon(QObject* parent, AddonType addonType, const QString& fileName,
             const QString& id, const QString& name, const QString& qml)
    : QObject(parent),
      m_addonType(addonType),
      m_fileName(fileName),
      m_id(id),
      m_name(name),
      m_qml(qml) {
  MVPN_COUNT_CTOR(Addon);

  QCoreApplication::installTranslator(&m_translator);
  retranslate();
}

Addon::~Addon() {
  MVPN_COUNT_DTOR(Addon);
  QCoreApplication::removeTranslator(&m_translator);
}

void Addon::retranslate() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString code = settingsHolder->languageCode();

  QLocale locale = QLocale(code);
  if (code.isEmpty()) {
    locale = QLocale(QLocale::system().bcp47Name());
  }

  if (!m_translator.load(locale, "locale", "_",
                         QString(":/addons/%1/i18n").arg(m_id))) {
    logger.error() << "Loading the locale failed. - code:" << code;
  }
}
