/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONGUIDE_H
#define ADDONGUIDE_H

#include "composer.h"
#include "addon.h"
#include "addonproperty.h"

class QJsonObject;

class AddonGuide final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonGuide)

 public:
  ADDON_PROPERTY(title, m_title, getTitle, setTitle, retranslationCompleted)
  ADDON_PROPERTY(subtitle, m_subtitle, getSubtitle, setSubtitle,
                 retranslationCompleted)

  Q_PROPERTY(QString image MEMBER m_image CONSTANT)
  Q_PROPERTY(Composer* composer MEMBER m_composer CONSTANT)
  Q_PROPERTY(bool advanced MEMBER m_advanced CONSTANT)

  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonGuide();

 private:
  AddonGuide(QObject* parent, const QString& manifestFileName,
             const QString& id, const QString& name);

 private:
  AddonProperty m_title;
  AddonProperty m_subtitle;
  QString m_image;
  bool m_advanced = false;

  Composer* m_composer = nullptr;
};

#endif  // ADDONGUIDE_H
