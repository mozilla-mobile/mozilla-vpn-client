/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONMESSAGE_H
#define ADDONMESSAGE_H

#include "addon.h"
#include "composer.h"

class QJsonObject;

class AddonMessage final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonMessage)

  Q_PROPERTY(QString titleId MEMBER m_titleId CONSTANT)
  Q_PROPERTY(Composer* composer MEMBER m_composer CONSTANT)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonMessage();

  Q_INVOKABLE void dismiss();

  bool enabled() const override;

 private:
  AddonMessage(QObject* parent, const QString& manifestFileName,
               const QString& id, const QString& name);

 private:
  QString m_titleId;
  Composer* m_composer = nullptr;

  bool m_dismissed = false;
};

#endif  // ADDONMESSAGE_H
