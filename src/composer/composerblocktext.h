/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKTEXT_H
#define COMPOSERBLOCKTEXT_H

#include "../addons/addonproperty.h"
#include "composerblock.h"

class ComposerBlockText final : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockText)
  QML_NAMED_ELEMENT(VPNComposerBlockText)
  QML_UNCREATABLE("")

 public:
  ADDON_PROPERTY(text, m_text, getText, setText, retranslationCompleted)

  static ComposerBlock* create(Composer* composer, const QString& blockId,
                               const QString& prefix, const QJsonObject& json);
  virtual ~ComposerBlockText();

  bool contains(const QString& string) const override;

 private:
  ComposerBlockText(Composer* composer, const QString& blockId);

 private:
  AddonProperty m_text;
};

#endif  // COMPOSERBLOCKTEXT_H
