/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "composerblockorderedlist.h"
#include "leakdetector.h"
#include "logger.h"

#include <QJsonArray>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ComposerBlockOrderedList");
}

// static
ComposerBlock* ComposerBlockOrderedList::create(Composer* composer,
                                                const QString& prefix,
                                                const QJsonObject& json) {
  ComposerBlockOrderedList* block = new ComposerBlockOrderedList(composer);
  if (!block->parseJson(prefix, json)) {
    block->deleteLater();
    return nullptr;
  }

  return block;
}

ComposerBlockOrderedList::ComposerBlockOrderedList(Composer* composer)
    : ComposerBlockUnorderedList(composer, "olist") {
  MVPN_COUNT_CTOR(ComposerBlockOrderedList);
}

ComposerBlockOrderedList::~ComposerBlockOrderedList() {
  MVPN_COUNT_DTOR(ComposerBlockOrderedList);
}
