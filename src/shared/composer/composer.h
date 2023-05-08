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
  QML_NAMED_ELEMENT(MZComposer)
  QML_UNCREATABLE("")

  Q_PROPERTY(QList<ComposerBlock*> blocks READ blocks NOTIFY blocksChanged)

 public:
  static Composer* create(Addon* addon, const QString& prefix,
                          const QJsonObject& obj);

  ~Composer();

  const QList<ComposerBlock*>& blocks() const { return m_blocks; }

  Q_INVOKABLE ComposerBlock* create(const QString& id, const QString& type,
                                    const QJSValue& params);
  Q_INVOKABLE void insert(int pos, ComposerBlock* block);
  Q_INVOKABLE void append(ComposerBlock* block);
  Q_INVOKABLE void remove(const QString& id);

 signals:
  void retranslationCompleted();
  void blocksChanged();

 private:
  Composer(Addon* addon, const QString& prefix);

 private:
  Addon* m_addon = nullptr;
  const QString m_prefix;

  QList<ComposerBlock*> m_blocks;
};

#endif  // COMPOSER_H
