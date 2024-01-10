/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonguide.h"

#include <QJsonObject>
#include <QScopeGuard>

#include "leakdetector.h"
#include "logging/logger.h"

namespace {
Logger logger("AddonGuide");
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

  guide->m_title.initialize(QString("guide.%1.title").arg(guideId),
                            guideObj["title"].toString());
  guide->m_subtitle.initialize(QString("guide.%1.subtitle").arg(guideId),
                               guideObj["subtitle"].toString());

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

  connect(guide, &Addon::retranslationCompleted, guide->m_composer,
          &Composer::retranslationCompleted);

  guard.dismiss();
  return guide;
}

AddonGuide::AddonGuide(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "guide") {
  MZ_COUNT_CTOR(AddonGuide);
}

AddonGuide::~AddonGuide() { MZ_COUNT_DTOR(AddonGuide); }
