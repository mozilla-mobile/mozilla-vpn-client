/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKTITLE_H
#define COMPOSERBLOCKTITLE_H

#include "../addons/addonproperty.h"
#include "composerblock.h"

class ComposerBlockTitle final : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockTitle)
  QML_NAMED_ELEMENT(VPNComposerBlockTitle)
  QML_UNCREATABLE("")

  ADDON_PROPERTY(title, m_title, retranslationCompleted)

 public:
  static ComposerBlock* create(Composer* composer, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlockTitle();

  bool contains(const QString& string) const override;

  QString getText() const override { return m_title.get(); }

 private:
  explicit ComposerBlockTitle(Composer* composer);

 private:
  AddonProperty m_title;
};

#endif  // COMPOSERBLOCKTITLE_H
