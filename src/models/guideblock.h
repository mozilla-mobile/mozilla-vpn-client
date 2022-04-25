/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GUIDEBLOCK_H
#define GUIDEBLOCK_H

#include "guidemodel.h"

class Guide;
class QJsonObject;

class GuideBlock final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(GuideBlock)

 public:
  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(GuideModel::GuideBlockType type MEMBER m_type CONSTANT)
  Q_PROPERTY(QStringList subBlockIds MEMBER m_subBlockIds CONSTANT)

  static GuideBlock* create(QObject* parent, const QString& guideId,
                            const QString& fileName, const QJsonObject& json);
  ~GuideBlock();

 private:
  explicit GuideBlock(QObject* parent);

 private:
  QString m_id;
  GuideModel::GuideBlockType m_type;
  QStringList m_subBlockIds;
};

#endif  // GUIDEBLOCK_H
