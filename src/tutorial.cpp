/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "inspector/inspectorutils.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"

#include <QCoreApplication>
#include <QFile>
#include <QQuickItem>

constexpr int TIMEOUT_ITEM_TIMER_MSEC = 300;

namespace {
Tutorial* s_instance = nullptr;
Logger logger(LOG_MAIN, "Tutorial");
}  // namespace

// static
Tutorial* Tutorial::instance() {
  if (!s_instance) {
    s_instance = new Tutorial(qApp);
  }
  return s_instance;
}

Tutorial::Tutorial(QObject* parent) : ItemPicker(parent) {
  MVPN_COUNT_CTOR(Tutorial);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() { processNextOp(); });
}

Tutorial::~Tutorial() { MVPN_COUNT_DTOR(Tutorial); }

void Tutorial::play(const QString& fileName) {
  if (!m_steps.isEmpty()) {
    m_steps.clear();
    emit playingChanged();
  }

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    emit playingChanged();
    return;
  }

  QByteArray content = file.readAll();
  QList<QByteArray> entries = content.split('\n');
  for (QByteArray& entry : entries) {
    QByteArray line = entry.trimmed();

    if (line.isEmpty()) continue;

    if (line[0] == '#') continue;

    if (line.startsWith("TOOLTIP ")) {
      QList<QByteArray> parts(line.remove(0, 8).split(' '));
      if (parts.length() != 2) {
        logger.error()
            << "Invalid tutorial file. TOOLTIP wants 2 elements only";
        return;
      }

      if (!L18nStrings::instance()->contains(parts[0])) {
        logger.error() << "Invalid tooltip string" << parts[0];
        return;
      }

      m_steps.append(Op{parts[1], parts[0]});
      continue;
    }

    logger.error() << "Invalid instruction" << line;
    m_steps.clear();
    return;
  }

  if (m_steps.isEmpty()) {
    logger.error() << "Empty tutorial";
    return;
  }

  emit playingChanged();

  qApp->installEventFilter(this);

  processNextOp();
}

void Tutorial::stop() {
  if (!isPlaying()) {
    return;
  }

  m_timer.stop();
  m_steps.clear();
  maybeStop();
}

bool Tutorial::maybeStop() {
  if (m_steps.isEmpty()) {
    qApp->removeEventFilter(this);
    setTooltipShown(false);
    emit playingChanged();
    return true;
  }

  return false;
}

void Tutorial::processNextOp() {
  if (maybeStop()) {
    return;
  }

  const Op& op = m_steps[0];
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
  Q_ASSERT(!m_steps.isEmpty());

  if (list.contains(m_steps[0].m_element)) {
    m_steps.removeFirst();
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
