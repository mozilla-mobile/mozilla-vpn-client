/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblock.h"
#include "composerblocktext.h"
#include "composerblocktitle.h"
#include "composerblockorderedlist.h"
#include "composerblockunorderedlist.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ComposerBlock");
}

ComposerBlock::ComposerBlock(QObject* parent, const QString& type)
    : QObject(parent), m_type(type) {
  MVPN_COUNT_CTOR(ComposerBlock);
}

ComposerBlock::~ComposerBlock() { MVPN_COUNT_DTOR(ComposerBlock); }

// static
ComposerBlock* ComposerBlock::create(QObject* parent, const QString& prefix,
                                     const QJsonObject& blockObj) {
  Q_ASSERT(parent);

  QString type = blockObj["type"].toString();
  if (type == "title") {
    return ComposerBlockTitle::create(parent, prefix, blockObj);
  }

  if (type == "text") {
    return ComposerBlockText::create(parent, prefix, blockObj);
  }

  if (type == "olist") {
    return ComposerBlockOrderedList::create(parent, prefix, blockObj);
  }

  if (type == "ulist") {
    return ComposerBlockUnorderedList::create(parent, prefix, blockObj);
  }

  logger.error() << "Invalid type for block for composer";
  return nullptr;
}
