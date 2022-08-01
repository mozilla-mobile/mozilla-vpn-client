/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCK_H
#define COMPOSERBLOCK_H

#include <QQmlEngine>

class QJsonObject;

class ComposerBlock : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlock)
  QML_NAMED_ELEMENT(VPNComposerBlock)
  QML_UNCREATABLE("")

  Q_PROPERTY(QString type MEMBER m_type CONSTANT)

 public:
  static ComposerBlock* create(QObject* parent, const QString& prefix,
                               const QJsonObject& json);
  virtual ~ComposerBlock();

 protected:
  explicit ComposerBlock(QObject* parent, const QString& type);

 private:
  const QString m_type;
};

#endif  // COMPOSERBLOCK_H
