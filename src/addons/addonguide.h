/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONGUIDE_H
#define ADDONGUIDE_H

#include "addon.h"
#include "composer.h"

class QJsonObject;

class AddonGuide final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonGuide)

  Q_PROPERTY(QString titleId MEMBER m_titleId CONSTANT)
  Q_PROPERTY(QString subtitleId MEMBER m_subtitleId CONSTANT)
  Q_PROPERTY(QString image MEMBER m_image CONSTANT)
  Q_PROPERTY(Composer* composer MEMBER m_composer CONSTANT)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonGuide();

 private:
  AddonGuide(QObject* parent, const QString& manifestFileName,
             const QString& id, const QString& name);

 private:
  QString m_titleId;
  QString m_subtitleId;
  QString m_image;
  Composer* m_composer = nullptr;
};

#endif  // ADDONGUIDE_H
