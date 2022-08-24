/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblocktext.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ComposerBlockText");
}

// static
ComposerBlock* ComposerBlockText::create(Composer* composer,
                                         const QString& prefix,
                                         const QJsonObject& json) {
  QString blockId = json["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for composer block text";
    return nullptr;
  }

  ComposerBlockText* block = new ComposerBlockText(composer);
  block->m_text.initialize(QString("%1.block.%2").arg(prefix).arg(blockId),
                           json["content"].toString());
  return block;
}

ComposerBlockText::ComposerBlockText(Composer* composer)
    : ComposerBlock(composer, "text") {
  MVPN_COUNT_CTOR(ComposerBlockText);
}

ComposerBlockText::~ComposerBlockText() { MVPN_COUNT_DTOR(ComposerBlockText); }
