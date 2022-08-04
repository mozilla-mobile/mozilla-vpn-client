/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONMESSAGE_H
#define ADDONMESSAGE_H

#include "addon.h"
#include "addonproperty.h"
#include "../composer/composer.h"

class QJsonObject;

class AddonMessage final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonMessage)

  ADDON_PROPERTY(title, m_title, retranslationCompleted)

  Q_PROPERTY(Composer* composer MEMBER m_composer CONSTANT)
  Q_PROPERTY(bool isRead MEMBER m_isRead NOTIFY isReadChanged)

 public:
  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonMessage();

  Q_INVOKABLE void dismiss();
  Q_INVOKABLE void maskAsRead();

  bool enabled() const override;

 signals:
  void isReadChanged();

 private:
  AddonMessage(QObject* parent, const QString& manifestFileName,
               const QString& id, const QString& name);

 private:
  AddonProperty m_title;
  Composer* m_composer = nullptr;

  bool m_dismissed = false;
  bool m_isRead = false;
};

#endif  // ADDONMESSAGE_H
