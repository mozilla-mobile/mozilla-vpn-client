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

  Q_PROPERTY(Composer* composer READ composer CONSTANT)
  Q_PROPERTY(bool isRead MEMBER m_isRead NOTIFY isReadChanged)
  Q_PROPERTY(qint64 date MEMBER m_date)
  Q_PROPERTY(QString formattedDate READ formattedDate NOTIFY retranslationCompleted)
  Q_PROPERTY(QString subtitle MEMBER m_subtitle NOTIFY subtitleChanged)

 public:
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
  void subtitleChanged();

 private:
  AddonMessage(QObject* parent, const QString& manifestFileName,
               const QString& id, const QString& name);

  void planDateRetranslation();

 private:
  AddonProperty m_title;
  Composer* m_composer = nullptr;

  qint64 m_date = 0;
  QString m_formattedDate = "";
  QString m_subtitle = "";

  bool m_dismissed = false;
  bool m_isRead = false;
};

#endif  // ADDONMESSAGE_H
