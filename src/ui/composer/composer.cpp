/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>

#include "addons/addon.h"
#include "composerblock.h"
#include "logger.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("Composer");

}  // namespace

Composer::Composer(Addon* addon, const QString& prefix)
    : QObject(addon), m_addon(addon), m_prefix(prefix) {
  MZ_COUNT_CTOR(Composer);
}

Composer::~Composer() { MZ_COUNT_DTOR(Composer); }

// static
Composer* Composer::create(Addon* addon, const QString& prefix,
                           const QJsonObject& obj) {
  Composer* composer = new Composer(addon, prefix);
  auto guard = qScopeGuard([&] { composer->deleteLater(); });

  QJsonValue blocksArray = obj["blocks"];
  if (!blocksArray.isArray()) {
    logger.warning() << "No blocks for composer";
    return nullptr;
  }

  const QJsonArray blocks = blocksArray.toArray();
  for (const QJsonValue& blockValue : blocks) {
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

ComposerBlock* Composer::create(const QString& id, const QString& type,
                                const QJSValue& params) {
  QJsonDocument json = QJsonDocument::fromVariant(params.toVariant());
  if (!json.isObject()) {
    return nullptr;
  }

  return ComposerBlock::create(this, m_addon, m_prefix, id, type,
                               json.object());
}

void Composer::insert(int pos, ComposerBlock* block) {
  if (m_blocks.contains(block)) {
    return;
  }

  if (pos >= 0 && pos <= m_blocks.length()) {
    m_blocks.insert(pos, block);
    emit blocksChanged();
  }
}

void Composer::append(ComposerBlock* block) {
  if (m_blocks.contains(block)) {
    return;
  }

  m_blocks.append(block);
  emit blocksChanged();
}

void Composer::remove(const QString& id) {
  for (int i = 0; i < m_blocks.length(); ++i) {
    if (m_blocks[i]->id() == id) {
      m_blocks.removeAt(i);
      break;
    }
  }

  emit blocksChanged();
}
