/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKUNORDEREDLIST_H
#define COMPOSERBLOCKUNORDEREDLIST_H

#include "../addons/addonpropertylist.h"
#include "composerblock.h"

class ComposerBlockUnorderedList : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockUnorderedList)
  QML_NAMED_ELEMENT(VPNComposerBlockUnorderedList)
  QML_UNCREATABLE("")

  ADDON_PROPERTY_LIST(subBlocks, m_subBlocks, retranslationCompleted)

 public:
  static ComposerBlock* create(Composer* composer, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlockUnorderedList();

  bool contains(const QString& string) const override;

 protected:
  ComposerBlockUnorderedList(Composer* composer, const QString& type);

  bool parseJson(const QString& prefix, const QJsonObject& json);

 private:
  AddonPropertyList m_subBlocks;
};

#endif  // COMPOSERBLOCKUNORDEREDLIST_H
