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
ComposerBlock* ComposerBlockOrderedList::create(QObject* parent,
                                                const QString& prefix,
                                                const QJsonObject& json) {
  QStringList subBlockIds;
  if (!parseJson(prefix, json, subBlockIds)) {
    return nullptr;
  }

  return new ComposerBlockOrderedList(parent, subBlockIds);
}

ComposerBlockOrderedList::ComposerBlockOrderedList(
    QObject* parent, const QStringList& subBlockIds)
    : ComposerBlockUnorderedList(parent, "olist", subBlockIds) {
  MVPN_COUNT_CTOR(ComposerBlockOrderedList);
}

ComposerBlockOrderedList::~ComposerBlockOrderedList() {
  MVPN_COUNT_DTOR(ComposerBlockOrderedList);
}
