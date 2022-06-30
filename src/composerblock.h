/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPOSERBLOCK_H
#define COMPOSERBLOCK_H

#include <QQmlEngine>

class QJsonObject;

class ComposerBlock final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ComposerBlock)
  QML_NAMED_ELEMENT(VPNComposerBlock)
  QML_UNCREATABLE("")

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(ComposerBlockType type MEMBER m_type CONSTANT)
  Q_PROPERTY(QStringList subBlockIds MEMBER m_subBlockIds CONSTANT)

 public:
  enum ComposerBlockType {
    ComposerBlockTypeTitle,
    ComposerBlockTypeText,
    ComposerBlockTypeOrderedList,
    ComposerBlockTypeUnorderedList,
  };
  Q_ENUM(ComposerBlockType);

  static ComposerBlock* create(QObject* parent, const QString& prefix,
                               const QJsonObject& json);
  ~ComposerBlock();

 private:
  explicit ComposerBlock(QObject* parent);

 private:
  QString m_id;
  ComposerBlockType m_type;
  QStringList m_subBlockIds;
};

#endif  // COMPOSERBLOCK_H
