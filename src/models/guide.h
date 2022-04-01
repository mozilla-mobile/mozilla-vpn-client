/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GUIDE_H
#define GUIDE_H

#include "guideblock.h"

class Guide final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Guide)

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString image MEMBER m_image CONSTANT)
  Q_PROPERTY(QList<GuideBlock*> blocks MEMBER m_blocks CONSTANT)

 public:
  static Guide* create(QObject* parent, const QString& fileName);

  static QString pascalize(const QString& input);

  ~Guide();

 private:
  explicit Guide(QObject* parent);

 private:
  QString m_id;
  QString m_image;
  QList<GuideBlock*> m_blocks;
};

#endif  // GUIDE_H
