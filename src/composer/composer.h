/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSER_H
#define COMPOSER_H

#include "composerblock.h"

class Addon;

class Composer final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Composer)
  QML_NAMED_ELEMENT(VPNComposer)
  QML_UNCREATABLE("")

  Q_PROPERTY(QList<ComposerBlock*> blocks READ blocks CONSTANT)

 public:
  static Composer* create(Addon* addon, const QString& prefix,
                          const QJsonObject& obj);

  ~Composer();

  const QList<ComposerBlock*>& blocks() const { return m_blocks; }

 signals:
  void retranslationCompleted();

 private:
  explicit Composer(QObject* parent);

 private:
  QList<ComposerBlock*> m_blocks;
};

#endif  // COMPOSER_H
