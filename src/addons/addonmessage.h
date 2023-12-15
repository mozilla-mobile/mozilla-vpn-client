/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONMESSAGE_H
#define ADDONMESSAGE_H

#include "../composer/composer.h"
#include "addon.h"
#include "addonproperty.h"
#include "settingsholder.h"

class QJsonObject;

#ifdef UNIT_TEST
class TestAddon;
#endif

constexpr const char* ADDON_MESSAGE_SETTINGS_GROUP = "message";
constexpr const char* ADDON_MESSAGE_SETTINGS_STATUS_KEY = "state";

class AddonMessage final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonMessage)
  QML_NAMED_ELEMENT(MZAddonMessage)
  QML_UNCREATABLE("")

 public:
  ADDON_PROPERTY(title, m_title, getTitle, setTitle, retranslationCompleted)
  ADDON_PROPERTY(subtitle, m_subtitle, getSubtitle, setSubtitle,
                 retranslationCompleted)

  Q_PROPERTY(Composer* composer READ composer CONSTANT)
  Q_PROPERTY(bool isRead READ isRead NOTIFY statusChanged)
  Q_PROPERTY(qint64 date MEMBER m_date WRITE setDate NOTIFY dateChanged)
  Q_PROPERTY(
      QString formattedDate READ formattedDate NOTIFY retranslationCompleted)
  Q_PROPERTY(Badge badge MEMBER m_badge WRITE setBadge NOTIFY badgeChanged)

  enum Badge {
    None,
    Warning,
    Critical,
    NewUpdate,
    WhatsNew,
    Survey,
    Subscription
  };
  Q_ENUM(Badge)

  static Addon* create(QObject* parent, const QString& manifestFileName,
                       const QString& id, const QString& name,
                       const QJsonObject& obj);

  ~AddonMessage();

  enum MessageStatus {
    // A message has been received
    Received,
    // A notification has been shown to the user i.e it's been enabled
    Notified,
    // Message has been read
    Read,
    // Message has been dismissed
    Dismissed
  };
  Q_ENUM(MessageStatus);

  Q_INVOKABLE void dismiss();
  Q_INVOKABLE void markAsRead();
  Q_INVOKABLE void resetMessage();
  Q_INVOKABLE bool containsSearchString(const QString& query) const;

  void setBadge(Badge badge);
  void setDate(qint64 date);

  bool isRead() const { return m_status == MessageStatus::Read; }

  bool isReceived() const { return m_status == MessageStatus::Received; }

  bool shouldNotify() const { return m_shouldNotify; }

  QString formattedDate() const;

  bool enabled() const override;

  Composer* composer() const { return m_composer; }

  void updateMessageStatus(MessageStatus newStatus);

  // Explosed for testing.
  static qint64 planDateRetranslationInternal(const QDateTime& nowDateTime,
                                              const QDateTime& messageDateTime);

 signals:
  void statusChanged(AddonMessage::MessageStatus status);
  void badgeChanged();
  void dateChanged();

 private:
  AddonMessage(QObject* parent, const QString& manifestFileName,
               const QString& id, const QString& name);

  MessageStatus loadMessageStatus(const QString& id);

  void planDateRetranslation();
  void setBadge(const QString& badge);

 private:
  AddonProperty m_title;
  AddonProperty m_subtitle;
  Composer* m_composer = nullptr;

  qint64 m_date = 0;
  bool m_shouldNotify = true;

  MessageStatus m_status = MessageStatus::Received;

  SettingGroup m_messageSettings;

  Badge m_badge;

#ifdef UNIT_TEST
  friend class TestAddon;
#endif
};

#endif  // ADDONMESSAGE_H
