/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblockunorderedlist.h"

#include <QJsonArray>
#include <QJsonObject>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("ComposerBlockUnorderedList");
}

// static
ComposerBlock* ComposerBlockUnorderedList::create(Composer* composer,
                                                  const QString& blockId,
                                                  const QString& prefix,
                                                  const QJsonObject& json) {
  ComposerBlockUnorderedList* block =
      new ComposerBlockUnorderedList(composer, blockId, "ulist");
  if (!block->parseJson(prefix, json)) {
    block->deleteLater();
    return nullptr;
  }

  return block;
}

bool ComposerBlockUnorderedList::parseJson(const QString& prefix,
                                           const QJsonObject& json) {
  QString blockId = json["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for composer block list";
    return false;
  }

  QJsonValue subBlockArray = json["content"];
  if (!subBlockArray.isArray()) {
    logger.error() << "No content for block type list in composer";
    return false;
  }

  const QJsonArray subBlocks = subBlockArray.toArray();
  for (const QJsonValue& subBlockValue : subBlocks) {
    if (!subBlockValue.isObject()) {
      logger.error()
          << "Expected JSON object for block content list in composer";
      return false;
    }

    QJsonObject subBlockObj = subBlockValue.toObject();
    QString subBlockId = subBlockObj["id"].toString();
    if (subBlockId.isEmpty()) {
      logger.error() << "Empty sub block ID for composer";
      return false;
    }

    m_subBlocks.append(
        QString("%1.block.%2.%3").arg(prefix, blockId, subBlockId),
        subBlockObj["content"].toString());
  }

  return true;
}

ComposerBlockUnorderedList::ComposerBlockUnorderedList(Composer* composer,
                                                       const QString& blockId,
                                                       const QString& type)
    : ComposerBlock(composer, blockId, type) {
  MZ_COUNT_CTOR(ComposerBlockUnorderedList);
}

ComposerBlockUnorderedList::~ComposerBlockUnorderedList() {
  MZ_COUNT_DTOR(ComposerBlockUnorderedList);
}

bool ComposerBlockUnorderedList::contains(const QString& string) const {
  for (const QString& item : m_subBlocks.get()) {
    if (item.contains(string, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}
