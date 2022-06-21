/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "guide.h"
#include "composer.h"
#include "leakdetector.h"
#include "logger.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>

namespace {
Logger logger(LOG_MAIN, "Guide");

}  // namespace

Guide::Guide(QObject* parent) : QObject(parent) { MVPN_COUNT_CTOR(Guide); }

Guide::~Guide() { MVPN_COUNT_DTOR(Guide); }

// static
Guide* Guide::create(QObject* parent, const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the guide file" << fileName;
    return nullptr;
  }

  QByteArray content = file.readAll();
  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.warning() << "Invalid JSON file" << fileName;
    return nullptr;
  }

  return create(parent, json.object());
}

// static
Guide* Guide::create(QObject* parent, const QJsonObject& obj) {
  QString guideId = obj["id"].toString();
  if (guideId.isEmpty()) {
    logger.warning() << "Empty ID for guide";
    return nullptr;
  }

  Guide* guide = new Guide(parent);
  auto guard = qScopeGuard([&] { guide->deleteLater(); });

  guide->m_titleId = QString("guide.%1.title").arg(guideId);
  guide->m_subtitleId = QString("guide.%1.subtitle").arg(guideId);

  guide->m_image = obj["image"].toString();
  if (guide->m_image.isEmpty()) {
    logger.warning() << "Empty image for guide";
    return nullptr;
  }

  guide->m_composer =
      Composer::create(guide, QString("guide.%1").arg(guideId), obj);
  if (!guide->m_composer) {
    logger.warning() << "Composer failed";
    return nullptr;
  }

  guard.dismiss();
  return guide;
}
