/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKTEXT_H
#define COMPOSERBLOCKTEXT_H

#include "composerblock.h"

class ComposerBlockText final : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockText)
  QML_NAMED_ELEMENT(VPNComposerBlockText)
  QML_UNCREATABLE("")

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)

 public:
  static ComposerBlock* create(QObject* parent, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlockText();

 private:
  ComposerBlockText(QObject* parent, const QString& id);

 private:
  QString m_id;
};

#endif  // COMPOSERBLOCKTEXT_H
