/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonguide.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonObject>
#include <QScopeGuard>

namespace {
Logger logger(LOG_MAIN, "AddonGuide");
}

// static
Addon* AddonGuide::create(QObject* parent, const QString& manifestFileName,
                          const QString& id, const QString& name,
                          const QJsonObject& obj) {
  QJsonObject guideObj = obj["guide"].toObject();

  QString guideId = guideObj["id"].toString();
  if (guideId.isEmpty()) {
    logger.warning() << "Empty ID for guide";
    return nullptr;
  }

  AddonGuide* guide = new AddonGuide(parent, manifestFileName, id, name);
  auto guard = qScopeGuard([&] { guide->deleteLater(); });

  guide->m_advanced = guideObj["advanced"].toBool();

  guide->m_titleId = QString("guide.%1.title").arg(guideId);
  guide->m_subtitleId = QString("guide.%1.subtitle").arg(guideId);

  guide->m_image = guideObj["image"].toString();
  if (guide->m_image.isEmpty()) {
    logger.warning() << "Empty image for guide";
    return nullptr;
  }

  guide->m_composer =
      Composer::create(guide, QString("guide.%1").arg(guideId), guideObj);
  if (!guide->m_composer) {
    logger.warning() << "Composer failed";
    return nullptr;
  }

  guard.dismiss();
  return guide;
}

AddonGuide::AddonGuide(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "guide") {
  MVPN_COUNT_CTOR(AddonGuide);
}

AddonGuide::~AddonGuide() { MVPN_COUNT_DTOR(AddonGuide); }
