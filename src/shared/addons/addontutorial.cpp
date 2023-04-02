/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addontutorial.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QScopeGuard>

#include "frontend/navigatorreloader.h"
#include "itempicker.h"
#include "leakdetector.h"
#include "logger.h"
#include "tutorial/tutorial.h"
#include "tutorial/tutorialstep.h"

namespace {
Logger logger("AddonTutorial");
}

class AddonTutorial::TutorialItemPicker final : public ItemPicker {
 public:
  TutorialItemPicker(AddonTutorial* parent)
      : ItemPicker(parent), m_parent(parent) {}

  void start() { qApp->installEventFilter(this); }

  void stop() { qApp->removeEventFilter(this); }

  bool itemPicked(const QList<QQuickItem*>& list) override {
    return m_parent->itemPicked(list);
  }

 private:
  AddonTutorial* m_parent = nullptr;
};

// static
Addon* AddonTutorial::create(QObject* parent, const QString& manifestFileName,
                             const QString& id, const QString& name,
                             const QJsonObject& obj) {
  QJsonObject tutorialObj = obj["tutorial"].toObject();

  QString tutorialId = tutorialObj["id"].toString();
  if (tutorialId.isEmpty()) {
    logger.warning() << "Empty ID for tutorial";
    return nullptr;
  }

  AddonTutorial* tutorial =
      new AddonTutorial(parent, manifestFileName, id, name);
  auto guard = qScopeGuard([&] { tutorial->deleteLater(); });

  tutorial->m_highlighted = tutorialObj["highlighted"].toBool();
  tutorial->m_settingsRollbackNeeded =
      tutorialObj["settings_rollback_needed"].toBool();

  tutorial->m_title.initialize(QString("tutorial.%1.title").arg(tutorialId),
                               tutorialObj["title"].toString());
  tutorial->m_subtitle.initialize(
      QString("tutorial.%1.subtitle").arg(tutorialId),
      tutorialObj["subtitle"].toString());
  tutorial->m_completionMessage.initialize(
      QString("tutorial.%1.completion_message").arg(tutorialId),
      tutorialObj["completion_message"].toString());

  tutorial->m_image = tutorialObj["image"].toString();
  if (tutorial->m_image.isEmpty()) {
    logger.warning() << "Empty image for tutorial";
    return nullptr;
  }

  QJsonValue stepsArray = tutorialObj["steps"];
  if (!stepsArray.isArray()) {
    logger.warning() << "No steps for tutorial";
    return nullptr;
  }

  const QJsonArray steps = stepsArray.toArray();
  for (QJsonValue stepValue : steps) {
    if (!stepValue.isObject()) {
      logger.warning() << "Expected JSON tutorialObjects as steps for tutorial";
      return nullptr;
    }

    TutorialStep* ts = TutorialStep::create(tutorial, tutorialId, stepValue);
    if (!ts) {
      logger.warning() << "Unable to create a tutorial step for tutorial";
      return nullptr;
    }

    tutorial->m_steps.append(ts);
  }

  if (tutorial->m_steps.isEmpty()) {
    logger.warning() << "Empty tutorial";
    return nullptr;
  }

  guard.dismiss();
  return tutorial;
}

AddonTutorial::AddonTutorial(QObject* parent, const QString& manifestFileName,
                             const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name),
      m_itemPicker(new TutorialItemPicker(this)) {
  MZ_COUNT_CTOR(AddonTutorial);
}

AddonTutorial::~AddonTutorial() { MZ_COUNT_DTOR(AddonTutorial); }

void AddonTutorial::play(const QStringList& allowedItems) {
  m_allowedItems = allowedItems;
  m_currentStep = 0;
  m_activeTransaction = false;

  if (settingsRollbackNeeded()) {
    m_activeTransaction = SettingsHolder::instance()->beginTransaction();
    if (!m_activeTransaction) {
      logger.warning() << "Unable to start a setting transaction";
    }
  }

  emit playingChanged();

  if (m_navigatorReloader) {
    m_navigatorReloader->deleteLater();
  }

  m_navigatorReloader = new NavigatorReloader(this);

  m_itemPicker->start();

  processNextOp();
}

void AddonTutorial::stop() {
  if (m_currentStep == -1) {
    return;
  }

  if (m_currentStep < m_steps.length()) {
    m_steps[m_currentStep]->stop();
  }

  if (m_activeTransaction &&
      !SettingsHolder::instance()->rollbackTransaction()) {
    logger.warning() << "Unable to rollback a setting transaction";
  }

  m_itemPicker->stop();
  m_currentStep = -1;
  m_activeTransaction = false;

  if (m_navigatorReloader) {
    m_navigatorReloader->deleteLater();
    m_navigatorReloader = nullptr;
  }

  emit playingChanged();
}

bool AddonTutorial::maybeStop() {
  if (m_currentStep != m_steps.length()) {
    return false;
  }

  Tutorial* tutorial = Tutorial::instance();
  Q_ASSERT(tutorial);

  tutorial->requireTutorialCompleted(this);

  Tutorial::instance()->stop();
  return true;
}

void AddonTutorial::processNextOp() {
  if (maybeStop()) {
    return;
  }

  Q_ASSERT(m_currentStep != -1 && m_currentStep < m_steps.length());
  connect(m_steps[m_currentStep], &TutorialStep::completed, this, [this]() {
    Q_ASSERT(m_currentStep > -1 && m_currentStep < m_steps.length());

    m_steps[m_currentStep]->stop();

    ++m_currentStep;
    processNextOp();
  });

  m_steps[m_currentStep]->start();
}

bool AddonTutorial::itemPicked(const QList<QQuickItem*>& list) {
  Q_ASSERT(m_currentStep != -1 && m_currentStep < m_steps.length());
  return m_steps[m_currentStep]->itemPicked(list);
}
