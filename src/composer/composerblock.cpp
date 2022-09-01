/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblock.h"
#include "composer.h"
#include "composerblockbutton.h"
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

ComposerBlock::ComposerBlock(Composer* composer, const QString& type)
    : QObject(composer), m_type(type) {
  MVPN_COUNT_CTOR(ComposerBlock);

  connect(composer, &Composer::retranslationCompleted, this,
          &ComposerBlock::retranslationCompleted);
}

ComposerBlock::~ComposerBlock() { MVPN_COUNT_DTOR(ComposerBlock); }

// static
ComposerBlock* ComposerBlock::create(Composer* composer, Addon* addon,
                                     const QString& prefix,
                                     const QJsonObject& blockObj) {
  Q_ASSERT(composer);

  QString type = blockObj["type"].toString();
  if (type == "title") {
    return ComposerBlockTitle::create(composer, prefix, blockObj);
  }

  if (type == "button") {
    return ComposerBlockButton::create(composer, addon, prefix, blockObj);
  }

  if (type == "text") {
    return ComposerBlockText::create(composer, prefix, blockObj);
  }

  if (type == "olist") {
    return ComposerBlockOrderedList::create(composer, prefix, blockObj);
  }

  if (type == "ulist") {
    return ComposerBlockUnorderedList::create(composer, prefix, blockObj);
  }

  logger.error() << "Invalid type for block for composer";
  return nullptr;
}
