/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addontutorial.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/tutorialmodel.h"

#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "AddonTutorial");
}

// static
Addon* AddonTutorial::create(QObject* parent, const QString& manifestFileName,
                             const QString& id, const QString& name,
                             const QJsonObject& obj) {
  if (!TutorialModel::instance()->createFromJson(id,
                                                 obj["tutorial"].toObject())) {
    logger.warning() << "Unable to add the tutorial";
    return nullptr;
  }

  return new AddonTutorial(parent, manifestFileName, id, name);
}

AddonTutorial::AddonTutorial(QObject* parent, const QString& manifestFileName,
                             const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "tutorial") {
  MVPN_COUNT_CTOR(AddonTutorial);
}

AddonTutorial::~AddonTutorial() {
  MVPN_COUNT_DTOR(AddonTutorial);

  TutorialModel::instance()->remove(id());
}
