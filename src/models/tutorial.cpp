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

  QString tutorialId = obj["id"].toString();
  if (tutorialId.isEmpty()) {
    logger.error() << "Empty ID for tutorial file" << fileName;
    return nullptr;
  }

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  Tutorial* tutorial = new Tutorial(parent);
  auto guard = qScopeGuard([&] { tutorial->deleteLater(); });

  tutorial->m_id =
      Guide::pascalize(QString("tutorial_%1_title").arg(tutorialId));
  if (!l18nStrings->contains(tutorial->m_id)) {
    logger.error() << "No string ID found for the title of tutorial file"
                   << fileName << "ID:" << tutorial->m_id;
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

    tutorial->m_steps.append(Op{element, stepId});
  }

  if (tutorial->m_steps.isEmpty()) {
    logger.error() << "Empty tutorial";
    return nullptr;
  }

  guard.dismiss();
  return tutorial;
}

void Tutorial::play() {
  m_currentStep = 0;

  emit playingChanged();

  qApp->installEventFilter(this);

  processNextOp();
}

void Tutorial::stop() {
  if (!isPlaying()) {
    return;
  }

  m_timer.stop();

  // Let's jump to the last one.
  m_currentStep = m_steps.length();
  maybeStop();
}

bool Tutorial::maybeStop() {
  if (m_currentStep == m_steps.length()) {
    qApp->removeEventFilter(this);
    setTooltipShown(false);
    m_currentStep = -1;
    emit playingChanged();
    return true;
  }

  return false;
}

void Tutorial::processNextOp() {
  if (maybeStop()) {
    return;
  }

  Q_ASSERT(m_currentStep != -1 && m_currentStep < m_steps.length());
  const Op& op = m_steps[m_currentStep];
  QObject* element = InspectorUtils::findObject(op.m_element);
  if (!element) {
    m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
    return;
  }

  QQuickItem* item = qobject_cast<QQuickItem*>(element);
  Q_ASSERT(item);

  QRectF rect = item->mapRectToScene(
      QRectF(item->x(), item->y(), item->width(), item->height()));

  setTooltipShown(true);
  emit tooltipNeeded(L18nStrings::instance()->value(op.m_stringId).toString(),
                     rect);
}

bool Tutorial::itemPicked(const QStringList& list) {
  Q_ASSERT(m_currentStep != -1 && m_currentStep < m_steps.length());

  if (list.contains(m_steps[m_currentStep].m_element)) {
    ++m_currentStep;
    processNextOp();
    return false;
  }

  for (const QString& objectName : m_allowedItems) {
    if (list.contains(objectName)) {
      return false;
    }
  }

  return true;
}

void Tutorial::allowItem(const QString& objectName) {
  m_allowedItems.append(objectName);
}

void Tutorial::setTooltipShown(bool shown) {
  m_tooltipShown = shown;
  emit tooltipShownChanged();
}
