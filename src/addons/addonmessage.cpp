/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmessage.h"

#include <QJsonObject>
#include <QMetaEnum>
#include <QTimer>

#include "glean/generated/metrics.h"
#include "i18nstrings.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger("AddonMessage");
}

// static
Addon* AddonMessage::create(QObject* parent, const QString& manifestFileName,
                            const QString& id, const QString& name,
                            const QJsonObject& obj) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  MessageStatus messageStatus = loadMessageStatus(id);
  if (messageStatus == MessageStatus::Dismissed) {
    logger.info() << "Message" << id << "has been already dismissed";
    return nullptr;
  }

  QJsonObject messageObj = obj["message"].toObject();

  QString messageId = messageObj["id"].toString();
  if (messageId.isEmpty()) {
    logger.warning() << "Empty ID for message";
    return nullptr;
  }

  AddonMessage* message = new AddonMessage(parent, manifestFileName, id, name);
  auto guard = qScopeGuard([&] { message->deleteLater(); });

  message->m_status = messageStatus;

  message->m_title.initialize(QString("message.%1.title").arg(messageId),
                              messageObj["title"].toString());

  message->m_subtitle.initialize(QString("message.%1.subtitle").arg(messageId),
                                 messageObj["subtitle"].toString());

  message->m_composer = Composer::create(
      message, QString("message.%1").arg(messageId), messageObj);
  if (!message->m_composer) {
    logger.warning() << "Composer failed";
    return nullptr;
  }

  message->m_date = messageObj["date"].toInteger();
  message->planDateRetranslation();

  message->setBadge(messageObj["badge"].toString());

  guard.dismiss();

  connect(message, &Addon::retranslationCompleted, message->m_composer,
          &Composer::retranslationCompleted);

  return message;
}

AddonMessage::AddonMessage(QObject* parent, const QString& manifestFileName,
                           const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "message") {
  MZ_COUNT_CTOR(AddonMessage);
}

AddonMessage::~AddonMessage() { MZ_COUNT_DTOR(AddonMessage); }

// static
AddonMessage::MessageStatus AddonMessage::loadMessageStatus(const QString& id) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString statusSetting =
      settingsHolder->getAddonSetting(MessageStatusQuery(id));
  QMetaEnum statusMetaEnum = QMetaEnum::fromType<MessageStatus>();

  bool isValidStatus = false;
  int persistedStatus = statusMetaEnum.keyToValue(
      statusSetting.toLocal8Bit().constData(), &isValidStatus);

  if (isValidStatus) {
    return static_cast<MessageStatus>(persistedStatus);
  }

  return MessageStatus::Received;
}

void AddonMessage::updateMessageStatus(MessageStatus newStatus) {
  if (m_status == newStatus) return;

  QMetaEnum statusMetaEnum = QMetaEnum::fromType<MessageStatus>();
  QString newStatusSetting = statusMetaEnum.valueToKey(newStatus);
  m_status = newStatus;
  emit statusChanged(m_status);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setAddonSetting(MessageStatusQuery(id()), newStatusSetting);

  mozilla::glean::sample::addon_message_state_changed.record(
      mozilla::glean::sample::AddonMessageStateChangedExtra{
          ._messageId = id(),
          ._messageState = newStatusSetting,
      });
}

void AddonMessage::dismiss() {
  disable();
  updateMessageStatus(MessageStatus::Dismissed);
}

void AddonMessage::markAsRead() { updateMessageStatus(MessageStatus::Read); }

bool AddonMessage::containsSearchString(const QString& query) const {
  if (query.isEmpty()) {
    return true;
  }

  if (m_title.get().contains(query, Qt::CaseInsensitive)) {
    return true;
  }

  if (m_subtitle.get().contains(query, Qt::CaseInsensitive)) {
    return true;
  }

  for (ComposerBlock* block : m_composer->blocks()) {
    if (block->contains(query)) {
      return true;
    }
  }
  return false;
}

bool AddonMessage::enabled() const {
  if (!Addon::enabled()) {
    return false;
  }

  return m_status != MessageStatus::Dismissed;
}

QString AddonMessage::formattedDate() const {
  if (m_date == 0) {
    return QString();
  }

  return Localizer::instance()->formatDate(
      QDateTime::currentDateTime(), QDateTime::fromSecsSinceEpoch(m_date),
      I18nStrings::instance()->t(I18nStrings::InAppMessagingDateTimeYesterday));
}

void AddonMessage::planDateRetranslation() {
  if (m_date == 0) {
    return;
  }

  qint64 time = planDateRetranslationInternal(
      QDateTime::currentDateTime(), QDateTime::fromSecsSinceEpoch(m_date));
  if (time == -1) {
    return;
  }

  QTimer::singleShot((1 + time) * 1000, this, [this]() {
    emit retranslationCompleted();
    planDateRetranslation();
  });
}

// static
qint64 AddonMessage::planDateRetranslationInternal(
    const QDateTime& nowDateTime, const QDateTime& messageDateTime) {
  qint64 diff = messageDateTime.secsTo(nowDateTime);

  qint64 secsTo = 0;
  if (diff < 0) {
    secsTo = nowDateTime.time().secsTo(QTime(0, 0));
  } else if (diff < 86400 && messageDateTime.time() <= nowDateTime.time()) {
    // Less than 24 hours ago, but still in the same day
    secsTo = messageDateTime.time().secsTo(QTime(0, 0));
  } else if (diff < 86400) {
    // Less than 24 hours ago
    secsTo = messageDateTime.time().secsTo(QTime(0, 0));
  } else {
    return -1;
  }

  if (secsTo > 0) {
    return secsTo;
  }

  return 86400 + secsTo;
}

void AddonMessage::setBadge(const QString& badge) {
  m_badge = None;
  if (badge.isEmpty() || badge == "none") {  // Nothing to do.
  } else if (badge == "warning") {
    m_badge = Warning;
  } else if (badge == "critical") {
    m_badge = Critical;
  } else if (badge == "new_update") {
    m_badge = NewUpdate;
  } else if (badge == "whats_new") {
    m_badge = WhatsNew;
  } else if (badge == "survey") {
    m_badge = Survey;
  } else {
    logger.error() << "Unsupported badge type" << badge;
  }
}

void AddonMessage::setBadge(Badge badge) {
  m_badge = badge;
  emit badgeChanged();
}

void AddonMessage::setDate(qint64 date) {
  m_date = date;
  emit dateChanged();
}
