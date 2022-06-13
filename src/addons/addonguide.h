/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONGUIDE_H
#define ADDONGUIDE_H

#include "addon.h"

class QJsonObject;

class AddonGuide final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonGuide)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonGuide();

 private:
  AddonGuide(QObject* parent, const QString& manifestFileName,
             const QString& id, const QString& name);
};

#endif  // ADDONGUIDE_H
