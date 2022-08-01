/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblockunorderedlist.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonArray>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ComposerBlockUnorderedList");
}

// static
ComposerBlock* ComposerBlockUnorderedList::create(QObject* parent,
                                                  const QString& prefix,
                                                  const QJsonObject& json) {
  QStringList subBlockIds;
  if (!parseJson(prefix, json, subBlockIds)) {
    return nullptr;
  }

  return new ComposerBlockUnorderedList(parent, "ulist", subBlockIds);
}

// static
bool ComposerBlockUnorderedList::parseJson(const QString& prefix,
                                           const QJsonObject& json,
                                           QStringList& subBlockIds) {
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

  for (QJsonValue subBlockValue : subBlockArray.toArray()) {
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

    subBlockIds.append(
        QString("%1.block.%2.%3").arg(prefix).arg(blockId).arg(subBlockId));
  }

  return true;
}

ComposerBlockUnorderedList::ComposerBlockUnorderedList(
    QObject* parent, const QString& type, const QStringList& subBlockIds)
    : ComposerBlock(parent, type), m_subBlockIds(subBlockIds) {
  MVPN_COUNT_CTOR(ComposerBlockUnorderedList);
}

ComposerBlockUnorderedList::~ComposerBlockUnorderedList() {
  MVPN_COUNT_DTOR(ComposerBlockUnorderedList);
}
