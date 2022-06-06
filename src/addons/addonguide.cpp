/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonguide.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/guidemodel.h"

#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "AddonGuide");
}

// static
Addon* AddonGuide::create(QObject* parent, const QString& manifestFileName,
                          const QString& id, const QString& name,
                          const QJsonObject& obj) {
  if (!GuideModel::instance()->createFromJson(id, obj["guide"].toObject())) {
    logger.warning() << "Unable to add the guide";
    return nullptr;
  }

  return new AddonGuide(parent, manifestFileName, id, name);
}

AddonGuide::AddonGuide(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name) {
  MVPN_COUNT_CTOR(AddonGuide);
}

AddonGuide::~AddonGuide() {
  MVPN_COUNT_DTOR(AddonGuide);

  GuideModel::instance()->remove(id());
}
