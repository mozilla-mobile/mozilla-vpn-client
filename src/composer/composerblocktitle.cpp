/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblocktitle.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ComposerBlockTitle");
}

// static
ComposerBlock* ComposerBlockTitle::create(Composer* composer,
                                          const QString& prefix,
                                          const QJsonObject& json) {
  QString blockId = json["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for composer block title";
    return nullptr;
  }

  ComposerBlockTitle* block = new ComposerBlockTitle(composer);
  block->m_title.initialize(QString("%1.block.%2").arg(prefix).arg(blockId),
                            json["content"].toString());
  return block;
}

ComposerBlockTitle::ComposerBlockTitle(Composer* composer)
    : ComposerBlock(composer, "title") {
  MVPN_COUNT_CTOR(ComposerBlockTitle);
}

ComposerBlockTitle::~ComposerBlockTitle() {
  MVPN_COUNT_DTOR(ComposerBlockTitle);
}
