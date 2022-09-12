/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKORDEREDLIST_H
#define COMPOSERBLOCKORDEREDLIST_H

#include "composerblockunorderedlist.h"

class ComposerBlockOrderedList final : public ComposerBlockUnorderedList {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockOrderedList)
  QML_NAMED_ELEMENT(VPNComposerBlockOrderedList)
  QML_UNCREATABLE("")

 public:
  static ComposerBlock* create(Composer* composer, const QString& blockId,
                               const QString& prefix, const QJsonObject& json);
  virtual ~ComposerBlockOrderedList();

 private:
  ComposerBlockOrderedList(Composer* composer, const QString& blockId);
};

#endif  // COMPOSERBLOCKORDEREDLIST_H
