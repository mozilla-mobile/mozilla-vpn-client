/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCKTITLE_H
#define COMPOSERBLOCKTITLE_H

#include "composerblock.h"

class ComposerBlockTitle final : public ComposerBlock {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlockTitle)
  QML_NAMED_ELEMENT(VPNComposerBlockTitle)
  QML_UNCREATABLE("")

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)

 public:
  static ComposerBlock* create(QObject* parent, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlockTitle();

 private:
  ComposerBlockTitle(QObject* parent, const QString& id);

 private:
  QString m_id;
};

#endif  // COMPOSERBLOCKTITLE_H
