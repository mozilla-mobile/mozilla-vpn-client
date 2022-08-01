/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblock.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonArray>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ComposerBlock");
}

ComposerBlock::ComposerBlock(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(ComposerBlock);
}

ComposerBlock::~ComposerBlock() { MVPN_COUNT_DTOR(ComposerBlock); }

// static
ComposerBlock* ComposerBlock::create(QObject* parent, const QString& prefix,
                                     const QJsonObject& blockObj) {
  Q_ASSERT(parent);

  ComposerBlock* block = new ComposerBlock(parent);

  QString blockId = blockObj["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for composer";
    return nullptr;
  }

  block->m_id = QString("%1.block.%2").arg(prefix).arg(blockId);

  QString type = blockObj["type"].toString();
  if (type == "title") {
    block->m_type = ComposerBlockTypeTitle;
  } else if (type == "text") {
    block->m_type = ComposerBlockTypeText;
  } else if (type == "olist") {
    block->m_type = ComposerBlockTypeOrderedList;
  } else if (type == "ulist") {
    block->m_type = ComposerBlockTypeUnorderedList;
  } else {
    logger.error() << "Invalid type for block for composer";
    return nullptr;
  }

  if (block->m_type == ComposerBlockTypeOrderedList ||
      block->m_type == ComposerBlockTypeUnorderedList) {
    QJsonValue subBlockArray = blockObj["content"];
    if (!subBlockArray.isArray()) {
      logger.error() << "No content for block type list in composer";
      return nullptr;
    }

    for (QJsonValue subBlockValue : subBlockArray.toArray()) {
      if (!subBlockValue.isObject()) {
        logger.error()
            << "Expected JSON object for block content list in composer";
        return nullptr;
      }

      QJsonObject subBlockObj = subBlockValue.toObject();
      QString subBlockId = subBlockObj["id"].toString();
      if (subBlockId.isEmpty()) {
        logger.error() << "Empty sub block ID for composer";
        return nullptr;
      }

      subBlockId =
          QString("%1.block.%2.%3").arg(prefix).arg(blockId).arg(subBlockId);
      block->m_subBlockIds.append(subBlockId);
    }
  }

  return block;
}
