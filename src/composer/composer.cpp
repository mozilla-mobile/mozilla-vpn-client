/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composer.h"
#include "addons/addon.h"
#include "composerblock.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QScopeGuard>

namespace {
Logger logger(LOG_MAIN, "Composer");

}  // namespace

Composer::Composer(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Composer);
}

Composer::~Composer() { MVPN_COUNT_DTOR(Composer); }

// static
Composer* Composer::create(Addon* addon, const QString& prefix,
                           const QJsonObject& obj) {
  Composer* composer = new Composer(addon);
  auto guard = qScopeGuard([&] { composer->deleteLater(); });

  QJsonValue blocksArray = obj["blocks"];
  if (!blocksArray.isArray()) {
    logger.warning() << "No blocks for composer";
    return nullptr;
  }

  for (QJsonValue blockValue : blocksArray.toArray()) {
    if (!blockValue.isObject()) {
      logger.warning() << "Expected JSON objects as blocks for composer";
      return nullptr;
    }

    QJsonObject blockObj = blockValue.toObject();

    ComposerBlock* block =
        ComposerBlock::create(composer, addon, prefix, blockObj);
    if (!block) {
      return nullptr;
    }

    composer->m_blocks.append(block);
  }

  guard.dismiss();
  return composer;
}
