/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "guideblock.h"
#include "guide.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonArray>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "GuideBlock");
}

GuideBlock::GuideBlock(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(GuideBlock);
}

GuideBlock::~GuideBlock() { MVPN_COUNT_DTOR(GuideBlock); }

// static
GuideBlock* GuideBlock::create(QObject* parent, const QString& guideId,
                               const QString& fileName,
                               const QJsonObject& blockObj) {
  Q_ASSERT(parent);

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  GuideBlock* block = new GuideBlock(parent);

  QString blockId = blockObj["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for guide file" << fileName;
    return nullptr;
  }

  block->m_id =
      Guide::pascalize(QString("guide_%1_block_%2").arg(guideId).arg(blockId));

  QString type = blockObj["type"].toString();
  if (type == "title") {
    block->m_type = GuideModel::GuideBlockTypeTitle;
  } else if (type == "text") {
    block->m_type = GuideModel::GuideBlockTypeText;
  } else if (type == "list") {
    block->m_type = GuideModel::GuideBlockTypeList;
  } else {
    logger.error() << "Invalid type for block for guide file" << fileName;
    return nullptr;
  }

  if (block->m_type == GuideModel::GuideBlockTypeList) {
    QJsonValue subBlockArray = blockObj["content"];
    if (!subBlockArray.isArray()) {
      logger.error() << "No content for block type list in guide file"
                     << fileName;
      return nullptr;
    }

    for (QJsonValue subBlockValue : subBlockArray.toArray()) {
      if (!subBlockValue.isObject()) {
        logger.error()
            << "Expected JSON object for block content list in guide file"
            << fileName;
        return nullptr;
      }

      QJsonObject subBlockObj = subBlockValue.toObject();
      QString subBlockId = subBlockObj["id"].toString();
      if (subBlockId.isEmpty()) {
        logger.error() << "Empty sub block ID for guide file" << fileName;
        return nullptr;
      }

      subBlockId = Guide::pascalize(QString("guide_%1_block_%2_%3")
                                        .arg(guideId)
                                        .arg(blockId)
                                        .arg(subBlockId));
      if (!l18nStrings->contains(subBlockId)) {
        logger.error() << "No string ID found for the block of guide file"
                       << fileName << "ID:" << subBlockId;
        return nullptr;
      }

      block->m_subBlockIds.append(subBlockId);
    }
  } else {
    if (!l18nStrings->contains(block->m_id)) {
      logger.error() << "No string ID found for the block of guide file"
                     << fileName << "ID:" << block->m_id;
      return nullptr;
    }
  }

  return block;
}
