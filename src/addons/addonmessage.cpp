/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmessage.h"
#include "constants.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "settingsholder.h"
#include "telemetry/gleansample.h"

#include <QJsonObject>
#include <QMetaEnum>

namespace {
Logger logger(LOG_MAIN, "AddonMessage");
}

// static
Addon* AddonMessage::create(QObject* parent, const QString& manifestFileName,
                            const QString& id, const QString& name,
                            const QJsonObject& obj) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  MessageState messageState = loadMessageState(id);
  if (messageState == MessageState::Dismissed) {
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

  message->m_state = messageState;

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
  MVPN_COUNT_CTOR(AddonMessage);
}

AddonMessage::~AddonMessage() { MVPN_COUNT_DTOR(AddonMessage); }

// static
AddonMessage::MessageState AddonMessage::loadMessageState(const QString& id) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString stateSetting = settingsHolder->getAddonSetting(MessageStateQuery(id));
  QMetaEnum stateMetaEnum = QMetaEnum::fromType<MessageState>();

  bool isValidState = false;
  int persistedState = stateMetaEnum.keyToValue(
      stateSetting.toLocal8Bit().constData(), &isValidState);

  if (isValidState) {
    return static_cast<MessageState>(persistedState);
  }

  return MessageState::Received;
}

void AddonMessage::updateMessageState(MessageState newState) {
  if (m_state == newState) return;

  QMetaEnum stateMetaEnum = QMetaEnum::fromType<MessageState>();
  QString newStateSetting = stateMetaEnum.valueToKey(newState);
  m_state = newState;
  emit stateChanged(m_state);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setAddonSetting(MessageStateQuery(id()), newStateSetting);
  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::addonMessageStateChanged,
      {{"message_id", id()}, {"message_state", newStateSetting}});
}

void AddonMessage::dismiss() {
  disable();
  updateMessageState(MessageState::Dismissed);
}

void AddonMessage::markAsRead() { updateMessageState(MessageState::Read); }

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

  return m_state != MessageState::Dismissed;
}

QString AddonMessage::formattedDate() const {
  if (m_date == 0) {
    return QString();
  }

  return dateInternal(QDateTime::currentDateTime(),
                      QDateTime::fromSecsSinceEpoch(m_date));
}

// static
QString AddonMessage::dateInternal(const QDateTime& nowDateTime,
                                   const QDateTime& messageDateTime) {
  qint64 diff = messageDateTime.secsTo(nowDateTime);
  if (diff < 0) {
    // The addon has a date set in the future...?
    return Localizer::instance()->locale().toString(nowDateTime.time(),
                                                    QLocale::ShortFormat);
  }

  // Today
  if (diff < 86400 && messageDateTime.time() <= nowDateTime.time()) {
    return Localizer::instance()->locale().toString(messageDateTime.time(),
                                                    QLocale::ShortFormat);
  }

  // Yesterday
  if (messageDateTime.date().dayOfYear() ==
          nowDateTime.date().dayOfYear() - 1 ||
      (nowDateTime.date().dayOfYear() == 1 &&
       messageDateTime.date().dayOfYear() ==
           messageDateTime.date().daysInYear())) {
    return L18nStrings::instance()->t(
        L18nStrings::InAppMessagingDateTimeYesterday);
  }

  // Before yesterday (but still this week)
  if (messageDateTime.date() >= nowDateTime.date().addDays(-6)) {
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    QString code = settingsHolder->languageCode();
    QLocale locale = QLocale(code);
    return locale.dayName(messageDateTime.date().dayOfWeek());
  }

  // Before this week
  return Localizer::instance()->locale().toString(messageDateTime.date(),
                                                  QLocale::ShortFormat);
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

void AddonMessage::enable() {
  Addon::enable();

  maybePushNotification();
}

void AddonMessage::maybePushNotification() {
  NotificationHandler* notificationHandler = NotificationHandler::instance();
  if (!notificationHandler) {
    return;
  }

  if (m_state == MessageState::Received) {
    NotificationHandler::instance()->newInAppMessageNotification(
        m_title.get(), m_subtitle.get());
    updateMessageState(MessageState::Notified);
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
