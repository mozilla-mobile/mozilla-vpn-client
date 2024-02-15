/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblocktitle.h"

#include <QJsonObject>

#include "logging/logger.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("ComposerBlockTitle");
}

// static
ComposerBlock* ComposerBlockTitle::create(Composer* composer,
                                          const QString& blockId,
                                          const QString& prefix,
                                          const QJsonObject& json) {
  ComposerBlockTitle* block = new ComposerBlockTitle(composer, blockId);
  block->m_title.initialize(QString("%1.block.%2").arg(prefix, blockId),
                            json["content"].toString());
  return block;
}

ComposerBlockTitle::ComposerBlockTitle(Composer* composer,
                                       const QString& blockId)
    : ComposerBlock(composer, blockId, "title") {
  MZ_COUNT_CTOR(ComposerBlockTitle);
}

ComposerBlockTitle::~ComposerBlockTitle() { MZ_COUNT_DTOR(ComposerBlockTitle); }

bool ComposerBlockTitle::contains(const QString& string) const {
  return m_title.get().contains(string, Qt::CaseInsensitive);
}
