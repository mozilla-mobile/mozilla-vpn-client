/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblockorderedlist.h"

#include <QJsonArray>
#include <QJsonObject>

#include "logger.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("ComposerBlockOrderedList");
}

// static
ComposerBlock* ComposerBlockOrderedList::create(Composer* composer,
                                                const QString& blockId,
                                                const QString& prefix,
                                                const QJsonObject& json) {
  ComposerBlockOrderedList* block =
      new ComposerBlockOrderedList(composer, blockId);
  if (!block->parseJson(prefix, json)) {
    block->deleteLater();
    return nullptr;
  }

  return block;
}

ComposerBlockOrderedList::ComposerBlockOrderedList(Composer* composer,
                                                   const QString& blockId)
    : ComposerBlockUnorderedList(composer, blockId, "olist") {
  MZ_COUNT_CTOR(ComposerBlockOrderedList);
}

ComposerBlockOrderedList::~ComposerBlockOrderedList() {
  MZ_COUNT_DTOR(ComposerBlockOrderedList);
}
