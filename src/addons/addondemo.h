/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONDEMO_H
#define ADDONDEMO_H

#include "addon.h"

class QJsonObject;

class AddonDemo final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonDemo)

  Q_PROPERTY(QString qml READ qml CONSTANT)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonDemo();

  QString qml() const;

 private:
  AddonDemo(QObject* parent, const QString& manifestFileName, const QString& id,
            const QString& name, const QString& qmlFileName);

 private:
  QString m_qmlFileName;
};

#endif  // ADDONDEMO_H
