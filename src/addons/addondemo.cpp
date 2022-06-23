/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addondemo.h"
#include "addonmanager.h"
#include "leakdetector.h"
#include "logger.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "AddonDemo");
}

// static
Addon* AddonDemo::create(QObject* parent, const QString& manifestFileName,
                         const QString& id, const QString& name,
                         const QJsonObject& obj) {
  QString qml = obj["qml"].toString();
  if (qml.isEmpty()) {
    logger.warning() << "No qml in the manifest" << manifestFileName;
    return nullptr;
  }

  QString qmlFileName = QFileInfo(manifestFileName).dir().filePath(qml);
  if (!QFile::exists(qmlFileName)) {
    logger.warning() << "Unable to load the qml entry" << qmlFileName << qml
                     << manifestFileName;
    return nullptr;
  }

  Addon* addon = new AddonDemo(parent, manifestFileName, id, name, qmlFileName);
  emit AddonManager::instance()->runAddon(addon);

  return addon;
}

AddonDemo::AddonDemo(QObject* parent, const QString& manifestFileName,
                     const QString& id, const QString& name,
                     const QString& qmlFileName)
    : Addon(parent, manifestFileName, id, name, "demo"),
      m_qmlFileName(qmlFileName) {
  MVPN_COUNT_CTOR(AddonDemo);
}

AddonDemo::~AddonDemo() { MVPN_COUNT_DTOR(AddonDemo); }

QString AddonDemo::qml() const {
  if (m_qmlFileName.at(0) == ':') {
    return QString("qrc%1").arg(m_qmlFileName);
  }

  return QString("file:%1").arg(m_qmlFileName);
}
