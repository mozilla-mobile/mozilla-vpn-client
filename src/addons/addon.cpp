/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addon.h"
#include "addondemo.h"
#include "addonguide.h"
#include "addoni18n.h"
#include "addontutorial.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/guidemodel.h"
#include "settingsholder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "Addon");
}

// static
Addon* Addon::create(QObject* parent, const QString& manifestFileName) {
  QFile file(manifestFileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the addon manifest of"
                     << manifestFileName;
    return nullptr;
  }

  QJsonDocument json = QJsonDocument::fromJson(file.readAll());
  if (!json.isObject()) {
    logger.warning() << "The manifest must be a JSON document"
                     << manifestFileName;
    return nullptr;
  }

  QJsonObject obj = json.object();

  QString version = obj["version"].toString();
  if (version.isEmpty()) {
    logger.warning() << "No version in the manifest" << manifestFileName;
    return nullptr;
  }

  if (version != "0.1") {
    logger.warning() << "Unsupported version" << version << manifestFileName;
    return nullptr;
  }

  QString id = obj["id"].toString();
  if (id.isEmpty()) {
    logger.warning() << "No id in the manifest" << manifestFileName;
    return nullptr;
  }

  QString name = obj["name"].toString();
  if (name.isEmpty()) {
    logger.warning() << "No name in the manifest" << manifestFileName;
    return nullptr;
  }

  QString type = obj["type"].toString();
  if (type.isEmpty()) {
    logger.warning() << "No type in the manifest" << manifestFileName;
    return nullptr;
  }

  if (type == "demo") {
    return AddonDemo::create(parent, manifestFileName, id, name, obj);
  }

  if (type == "i18n") {
    return new AddonI18n(parent, manifestFileName, id, name);
  }

  if (type == "tutorial") {
    return AddonTutorial::create(parent, manifestFileName, id, name, obj);
  }

  if (type == "guide") {
    return AddonGuide::create(parent, manifestFileName, id, name, obj);
  }

  logger.warning() << "Unsupported type" << type << manifestFileName;
  return nullptr;
}

Addon::Addon(QObject* parent, const QString& manifestFileName,
             const QString& id, const QString& name)
    : QObject(parent),
      m_manifestFileName(manifestFileName),
      m_id(id),
      m_name(name) {
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

  if (!m_translator.load(
          locale, "locale", "_",
          QFileInfo(m_manifestFileName).dir().filePath("i18n"))) {
    logger.error() << "Loading the locale failed. - code:" << code;
  }
}
