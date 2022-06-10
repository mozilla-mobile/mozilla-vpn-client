/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONTUTORIAL_H
#define ADDONTUTORIAL_H

#include "addon.h"

class QJsonObject;

class AddonTutorial final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonTutorial)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonTutorial();

 private:
  AddonTutorial(QObject* parent, const QString& manifestFileName,
                const QString& id, const QString& name);
};

#endif  // ADDONTUTORIAL_H
