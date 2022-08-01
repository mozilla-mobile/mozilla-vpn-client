/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKUNORDEREDLIST_H
#define COMPOSERBLOCKUNORDEREDLIST_H

#include "composerblock.h"

class ComposerBlockUnorderedList : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockUnorderedList)
  QML_NAMED_ELEMENT(VPNComposerBlockUnorderedList)
  QML_UNCREATABLE("")

  Q_PROPERTY(QStringList subBlockIds MEMBER m_subBlockIds CONSTANT)

 public:
  static ComposerBlock* create(QObject* parent, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlockUnorderedList();

 protected:
  ComposerBlockUnorderedList(QObject* parent, const QString& type,
                             const QStringList& subBlockIds);

  static bool parseJson(const QString& prefix, const QJsonObject& json,
                        QStringList& subBlockIds);

 private:
  QStringList m_subBlockIds;
};

#endif  // COMPOSERBLOCKUNORDEREDLIST_H
