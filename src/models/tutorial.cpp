/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "guide.h"
#include "inspector/inspectorutils.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "tutorialmodel.h"
#include "tutorialnext.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickItem>
#include <QScopeGuard>

constexpr int TIMEOUT_ITEM_TIMER_MSEC = 300;

namespace {
Logger logger(LOG_MAIN, "Tutorial");
}  // namespace

Tutorial::Tutorial(QObject* parent) : ItemPicker(parent) {
  MVPN_COUNT_CTOR(Tutorial);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() { processNextOp(); });
}

Tutorial::~Tutorial() { MVPN_COUNT_DTOR(Tutorial); }

// static
Tutorial* Tutorial::create(QObject* parent, const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.error() << "Unable to read the tutorial file" << fileName;
    return nullptr;
  }

  QByteArray content = file.readAll();
  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.error() << "Invalid JSON file" << fileName;
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
    logger.error() << "Empty ID for tutorial file" << fileName;
    return nullptr;
  }

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  Tutorial* tutorial = new Tutorial(parent);
  auto guard = qScopeGuard([&] { tutorial->deleteLater(); });

  tutorial->m_titleId =
      Guide::pascalize(QString("tutorial_%1_title").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_titleId)) {
    logger.error() << "No string ID found for the title of tutorial file"
                   << fileName << "ID:" << tutorial->m_titleId;
    return nullptr;
  }

  tutorial->m_subtitleId =
      Guide::pascalize(QString("tutorial_%1_subtitle").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_subtitleId)) {
    logger.error() << "No string ID found for the subtitle of tutorial file"
                   << fileName << "ID:" << tutorial->m_subtitleId;
    return nullptr;
  }

  tutorial->m_completionMessageId = Guide::pascalize(
      QString("tutorial_%1_completion_message").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_completionMessageId)) {
    logger.error()
        << "No string ID found for the completion message of tutorial file"
        << fileName << "ID:" << tutorial->m_completionMessageId;
    return nullptr;
  }

  tutorial->m_image = obj["image"].toString();
  if (tutorial->m_image.isEmpty()) {
    logger.error() << "Empty image for tutorial file" << fileName;
    return nullptr;
  }

  QJsonValue stepsArray = obj["steps"];
  if (!stepsArray.isArray()) {
    logger.error() << "No steps for tutorial file" << fileName;
    return nullptr;
  }

  for (QJsonValue stepValue : stepsArray.toArray()) {
    if (!stepValue.isObject()) {
      logger.error() << "Expected JSON objects as steps for tutorial file"
                     << fileName;
      return nullptr;
    }

    QJsonObject stepObj = stepValue.toObject();

    QString stepId = stepObj["id"].toString();
    if (stepId.isEmpty()) {
      logger.error() << "Empty ID step in tutorial file" << fileName;
      return nullptr;
    }

    stepId = Guide::pascalize(
        QString("tutorial_%1_step_%2").arg(tutorialId).arg(stepId));
    if (!l18nStrings->contains(stepId)) {
      logger.error() << "No string ID found for the step of tutorial file"
                     << fileName << "ID:" << stepId;
      return nullptr;
    }

    QString element = stepObj["element"].toString();
    if (element.isEmpty()) {
      logger.error() << "Empty element for step in tutorial file" << fileName;
      return nullptr;
    }

    QJsonObject conditions = stepObj["conditions"].toObject();

    TutorialNext* tn = TutorialNext::create(tutorial, stepObj["next"]);
    if (!tn) {
      logger.error() << "Unable to parse the 'next' property" << fileName;
      return nullptr;
    }

    tutorial->m_steps.append(Op{element, stepId, conditions, tn});
  }

  if (tutorial->m_steps.isEmpty()) {
    logger.error() << "Empty tutorial";
    return nullptr;
  }

  guard.dismiss();
  return tutorial;
}

void Tutorial::play(const QStringList& allowedItems) {
  m_allowedItems = allowedItems;
  m_currentStep = 0;
  m_elementPicked = false;

  qApp->installEventFilter(this);

  processNextOp();
}

void Tutorial::stop() {
  if (m_currentStep == -1) {
    return;
  }

  m_timer.stop();

  if (m_currentStep < m_steps.length()) {
    m_steps[m_currentStep].m_next->disconnect();
    m_steps[m_currentStep].m_next->stop();
  }

  qApp->removeEventFilter(this);
  m_currentStep = -1;
  m_elementPicked = false;
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
  const Op& op = m_steps[m_currentStep];

  if (!Guide::evaluateConditions(op.m_conditions)) {
    logger.info()
        << "Exclude the tutorial step because conditions do not match";
    ++m_currentStep;
    processNextOp();
    return;
  }

  QObject* element = InspectorUtils::findObject(op.m_element);
  if (!element) {
    m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
    return;
  }

  QQuickItem* item = qobject_cast<QQuickItem*>(element);
  Q_ASSERT(item);

  if (!item->isVisible()) {
    m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
    return;
  }

  // mapRectToScene/Item do not return the correct value. Let's compute the x/y
  // values manually.

  qreal x = item->x();
  qreal y = item->y();
  for (QQuickItem* parent = item->parentItem(); parent;
       parent = parent->parentItem()) {
    x += parent->x();
    y += parent->y();
  }

  TutorialModel* tutorialModel = TutorialModel::instance();
  Q_ASSERT(tutorialModel);

  tutorialModel->requireTooltipShown(this, true);
  tutorialModel->requireTooltipNeeded(
      this, L18nStrings::instance()->value(op.m_stringId).toString(),
      QRectF(x, y, item->width(), item->height()));

  connect(op.m_next, &TutorialNext::completed, this, [this]() {
    Q_ASSERT(m_currentStep > -1 && m_currentStep < m_steps.length());

    m_steps[m_currentStep].m_next->disconnect();
    m_steps[m_currentStep].m_next->stop();

    m_elementPicked = false;
    ++m_currentStep;
    processNextOp();
  });

  op.m_next->start();
}

bool Tutorial::itemPicked(const QList<QQuickItem*>& list) {
  Q_ASSERT(m_currentStep != -1 && m_currentStep < m_steps.length());

  const Op& op = m_steps[m_currentStep];
  QObject* element = InspectorUtils::findObject(op.m_element);
  if (element) {
    QQuickItem* item = qobject_cast<QQuickItem*>(element);
    Q_ASSERT(item);

    if (list.contains(item)) {
      m_elementPicked = true;
      return false;
    }
  }

  for (QQuickItem* item : list) {
    QString objectName = item->objectName();
    if (objectName.isEmpty()) continue;

    if (m_allowedItems.contains(objectName)) {
      return false;
    }
  }

  return true;
}
