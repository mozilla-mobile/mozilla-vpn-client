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
  QML_NAMED_ELEMENT(VPNAddonMessage)
  QML_UNCREATABLE("")

  ADDON_PROPERTY(title, m_title, retranslationCompleted)
  ADDON_PROPERTY(subtitle, m_subtitle, retranslationCompleted)

  Q_PROPERTY(Composer* composer READ composer CONSTANT)
  Q_PROPERTY(bool isRead MEMBER m_isRead NOTIFY isReadChanged)
  Q_PROPERTY(qint64 date MEMBER m_date)
  Q_PROPERTY(
      QString formattedDate READ formattedDate NOTIFY retranslationCompleted)
  Q_PROPERTY(Badge badge MEMBER m_badge CONSTANT)

 public:
  enum Badge { None, Warning, Critical, NewUpdate, WhatsNew, Survey };
  Q_ENUM(Badge)

  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonMessage();

  Q_INVOKABLE void dismiss();
  Q_INVOKABLE void maskAsRead();
  Q_INVOKABLE bool containsSearchString(const QString& query) const;

  QString formattedDate() const;

  bool enabled() const override;

  Composer* composer() const { return m_composer; }

  // Explosed for testing.
  static QString dateInternal(const QDateTime& nowDateTime,
                              const QDateTime& messageDateTime);
  static qint64 planDateRetranslationInternal(const QDateTime& nowDateTime,
                                              const QDateTime& messageDateTime);
 signals:
  void isReadChanged();

 private:
  AddonMessage(QObject* parent, const QString& manifestFileName,
               const QString& id, const QString& name);

  void planDateRetranslation();
  void setBadge(const QString& badge);

 private:
  AddonProperty m_title;
  AddonProperty m_subtitle;
  Composer* m_composer = nullptr;

  qint64 m_date = 0;

  bool m_dismissed = false;
  bool m_isRead = false;
  Badge m_badge;
};

#endif  // ADDONMESSAGE_H
