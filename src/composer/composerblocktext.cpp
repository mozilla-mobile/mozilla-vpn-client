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
                                         const QString& blockId,
                                         const QString& prefix,
                                         const QJsonObject& json) {
  ComposerBlockText* block = new ComposerBlockText(composer, blockId);
  block->m_text.initialize(QString("%1.block.%2").arg(prefix).arg(blockId),
                           json["content"].toString());
  return block;
}

ComposerBlockText::ComposerBlockText(Composer* composer, const QString& blockId)
    : ComposerBlock(composer, blockId, "text") {
  MVPN_COUNT_CTOR(ComposerBlockText);
}

ComposerBlockText::~ComposerBlockText() { MVPN_COUNT_DTOR(ComposerBlockText); }

bool ComposerBlockText::contains(const QString& string) const {
  return m_text.get().contains(string, Qt::CaseInsensitive);
}
