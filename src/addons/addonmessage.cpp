/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmessage.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "AddonMessage");
}

// static
Addon* AddonMessage::create(QObject* parent, const QString& manifestFileName,
                            const QString& id, const QString& name,
                            const QJsonObject& obj) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QStringList dismissedAddonMessages = settingsHolder->dismissedAddonMessages();
  if (dismissedAddonMessages.contains(id)) {
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

  message->m_titleId = QString("message.%1.title").arg(messageId);

  message->m_composer = Composer::create(
      message, QString("message.%1").arg(messageId), messageObj);
  if (!message->m_composer) {
    logger.warning() << "Composer failed";
    return nullptr;
  }

  guard.dismiss();
  return message;
}

AddonMessage::AddonMessage(QObject* parent, const QString& manifestFileName,
                           const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "message") {
  MVPN_COUNT_CTOR(AddonMessage);
}

AddonMessage::~AddonMessage() { MVPN_COUNT_DTOR(AddonMessage); }

void AddonMessage::dismiss() {
  m_dismissed = true;
  disable();

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QStringList dismissedAddonMessages = settingsHolder->dismissedAddonMessages();
  dismissedAddonMessages.append(id());
  settingsHolder->setDismissedAddonMessages(dismissedAddonMessages);
}

bool AddonMessage::enabled() const {
  if (!Addon::enabled()) {
    return false;
  }

  return !m_dismissed;
}
