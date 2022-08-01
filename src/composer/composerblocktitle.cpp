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
ComposerBlock* ComposerBlockTitle::create(QObject* parent,
                                          const QString& prefix,
                                          const QJsonObject& json) {
  QString blockId = json["id"].toString();
  if (blockId.isEmpty()) {
    logger.error() << "Empty block ID for composer block title";
    return nullptr;
  }

  return new ComposerBlockTitle(
      parent, QString("%1.block.%2").arg(prefix).arg(blockId));
}

ComposerBlockTitle::ComposerBlockTitle(QObject* parent, const QString& id)
    : ComposerBlock(parent, "title"), m_id(id) {
  MVPN_COUNT_CTOR(ComposerBlockTitle);
}

ComposerBlockTitle::~ComposerBlockTitle() {
  MVPN_COUNT_DTOR(ComposerBlockTitle);
}
