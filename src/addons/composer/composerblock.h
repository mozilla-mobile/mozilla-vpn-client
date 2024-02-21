/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCK_H
#define COMPOSERBLOCK_H

#include <QQmlEngine>

class Addon;
class Composer;
class QJsonObject;

class ComposerBlock : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlock)
  QML_NAMED_ELEMENT(MZComposerBlock)
  QML_UNCREATABLE("")

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString type MEMBER m_type CONSTANT)

 public:
  static ComposerBlock* create(Composer* composer, Addon* addon,
                               const QString& prefix, const QJsonObject& json);
  static ComposerBlock* create(Composer* composer, Addon* addon,
                               const QString& prefix, const QString& blockId,
                               const QString& type, const QJsonObject& json);
  virtual ~ComposerBlock();

  virtual bool contains(const QString& string) const = 0;

  const QString& id() const { return m_id; }

 signals:
  void retranslationCompleted();

 protected:
  explicit ComposerBlock(Composer* composer, const QString& id,
                         const QString& type);

 private:
  const QString m_id;
  const QString m_type;
};

#endif  // COMPOSERBLOCK_H
