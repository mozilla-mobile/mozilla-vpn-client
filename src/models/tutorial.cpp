/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "guide.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "tutorialmodel.h"
#include "tutorialstep.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>

namespace {
Logger logger(LOG_MAIN, "Tutorial");
}  // namespace

Tutorial::Tutorial(QObject* parent) : ItemPicker(parent) {
  MVPN_COUNT_CTOR(Tutorial);
}

Tutorial::~Tutorial() { MVPN_COUNT_DTOR(Tutorial); }

// static
Tutorial* Tutorial::create(QObject* parent, const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the tutorial file" << fileName;
    return nullptr;
  }

  QByteArray content = file.readAll();
  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.warning() << "Invalid JSON file" << fileName;
    return nullptr;
  }

  QJsonObject obj = json.object();

  QJsonObject conditions = obj["conditions"].toObject();
  if (!Guide::evaluateConditions(conditions)) {
    logger.info() << "Exclude the tutorial because conditions do not match";
    return nullptr;
  }

  QString tutorialId = obj["id"].toString();
  if (tutorialId.isEmpty()) {
    logger.warning() << "Empty ID for tutorial file" << fileName;
    return nullptr;
  }

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  Tutorial* tutorial = new Tutorial(parent);
  auto guard = qScopeGuard([&] { tutorial->deleteLater(); });

  tutorial->m_highlighted = obj["highlighted"].toBool();

  tutorial->m_titleId =
      Guide::pascalize(QString("tutorial_%1_title").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_titleId)) {
    logger.warning() << "No string ID found for the title of tutorial file"
                     << fileName << "ID:" << tutorial->m_titleId;
    return nullptr;
  }

  tutorial->m_subtitleId =
      Guide::pascalize(QString("tutorial_%1_subtitle").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_subtitleId)) {
    logger.warning() << "No string ID found for the subtitle of tutorial file"
                     << fileName << "ID:" << tutorial->m_subtitleId;
    return nullptr;
  }

  tutorial->m_completionMessageId = Guide::pascalize(
      QString("tutorial_%1_completion_message").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_completionMessageId)) {
    logger.warning()
        << "No string ID found for the completion message of tutorial file"
        << fileName << "ID:" << tutorial->m_completionMessageId;
    return nullptr;
  }

  tutorial->m_image = obj["image"].toString();
  if (tutorial->m_image.isEmpty()) {
    logger.warning() << "Empty image for tutorial file" << fileName;
    return nullptr;
  }

  QJsonValue stepsArray = obj["steps"];
  if (!stepsArray.isArray()) {
    logger.warning() << "No steps for tutorial file" << fileName;
    return nullptr;
  }

  for (QJsonValue stepValue : stepsArray.toArray()) {
    if (!stepValue.isObject()) {
      logger.warning() << "Expected JSON objects as steps for tutorial file"
                       << fileName;
      return nullptr;
    }

    TutorialStep* ts = TutorialStep::create(tutorial, tutorialId, stepValue);
    if (!ts) {
      logger.warning() << "Unable to create a tutorial step for tutorial file"
                       << fileName;
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

void Tutorial::play(const QStringList& allowedItems) {
  m_allowedItems = allowedItems;
  m_currentStep = 0;

  qApp->installEventFilter(this);

  processNextOp();
}

void Tutorial::stop() {
  if (m_currentStep == -1) {
    return;
  }

  if (m_currentStep < m_steps.length()) {
    m_steps[m_currentStep]->stop();
  }

  qApp->removeEventFilter(this);
  m_currentStep = -1;
}

bool Tutorial::maybeStop(bool completed) {
  if (m_currentStep != m_steps.length()) {
    return false;
  }

  if (completed) {
    TutorialModel* tutorialModel = TutorialModel::instance();
    Q_ASSERT(tutorialModel);

    tutorialModel->requireTutorialCompleted(
        this, L18nStrings::instance()->value(m_completionMessageId).toString());
  }

  TutorialModel::instance()->stop();
  return true;
}

void Tutorial::processNextOp() {
  if (maybeStop(true)) {
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

bool Tutorial::itemPicked(const QList<QQuickItem*>& list) {
  Q_ASSERT(m_currentStep != -1 && m_currentStep < m_steps.length());
  return m_steps[m_currentStep]->itemPicked(list);
}
