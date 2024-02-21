/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblock.h"

#include <QJsonObject>

#include "composer.h"
#include "composerblockbutton.h"
#include "composerblockorderedlist.h"
#include "composerblocktext.h"
#include "composerblocktitle.h"
#include "composerblockunorderedlist.h"
#include "logging/logger.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("ComposerBlock");
}

ComposerBlock::ComposerBlock(Composer* composer, const QString& id,
                             const QString& type)
    : QObject(composer), m_id(id), m_type(type) {
  MZ_COUNT_CTOR(ComposerBlock);

  connect(composer, &Composer::retranslationCompleted, this,
          &ComposerBlock::retranslationCompleted);
}

ComposerBlock::~ComposerBlock() { MZ_COUNT_DTOR(ComposerBlock); }

// static
ComposerBlock* ComposerBlock::create(Composer* composer, Addon* addon,
                                     const QString& prefix,
                                     const QJsonObject& blockObj) {
  Q_ASSERT(composer);

  QString blockId = blockObj["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for composer block";
    return nullptr;
  }

  QString type = blockObj["type"].toString();
  return create(composer, addon, prefix, blockId, type, blockObj);
}

// static
ComposerBlock* ComposerBlock::create(Composer* composer, Addon* addon,
                                     const QString& prefix,
                                     const QString& blockId,
                                     const QString& type,
                                     const QJsonObject& blockObj) {
  if (type == "title") {
    return ComposerBlockTitle::create(composer, blockId, prefix, blockObj);
  }

  if (type == "button") {
    return ComposerBlockButton::create(composer, addon, blockId, prefix,
                                       blockObj);
  }

  if (type == "text") {
    return ComposerBlockText::create(composer, blockId, prefix, blockObj);
  }

  if (type == "olist") {
    return ComposerBlockOrderedList::create(composer, blockId, prefix,
                                            blockObj);
  }

  if (type == "ulist") {
    return ComposerBlockUnorderedList::create(composer, blockId, prefix,
                                              blockObj);
  }

  logger.error() << "Invalid type for block for composer";
  return nullptr;
}
