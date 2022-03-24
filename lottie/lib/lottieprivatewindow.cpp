/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottieprivatewindow.h"
#include "lottieprivate.h"

#include <QJSEngine>
#include <QTimer>

LottiePrivateWindow::LottiePrivateWindow(LottiePrivate* parent)
    : QObject(parent), m_private(parent) {
  Q_ASSERT(parent);
}

int LottiePrivateWindow::setIntervalOrTimeout(QJSValue callback, int interval,
                                              bool singleShot) {
  int timerId = ++m_timerId;

  // No too aggressive animations.
  if (interval == 0) {
    interval = 100;
  }

  if (!callback.isCallable()) {
    return timerId;
  }

  TimerData td{new QTimer(this), callback, timerId, interval, singleShot};
  connect(td.m_timer, &QTimer::timeout, this, [this, timerId] {
    TimerData td = m_timers.value(timerId);
    if (td.m_singleShot) {
      clearInterval(timerId);
    } else {
      td.m_timer->start(td.m_interval);
    }
    td.m_callback.call();
  });

  td.m_timer->start(interval);
  td.m_timer->setSingleShot(true);

  m_timers.insert(td.m_timerId, td);

  return timerId;
}

void LottiePrivateWindow::clearInterval(int id) {
  TimerData td = m_timers.value(id);
  m_timers.remove(id);

  if (td.m_timer) {
    td.m_timer->deleteLater();
  }
}

QJSValue LottiePrivateWindow::requestAnimationFrame(QJSValue callback) {
  QJSValue canvasValue =
      m_private->engine()->toScriptValue(m_private->canvas());
  QJSValue requestAnimationFrameFunction =
      canvasValue.property("requestAnimationFrame");
  Q_ASSERT(requestAnimationFrameFunction.isCallable());
  return requestAnimationFrameFunction.callWithInstance(
      canvasValue, QList<QJSValue>{callback});
}

QJSValue LottiePrivateWindow::cancelAnimationFrame(int id) {
  QJSValue canvasValue =
      m_private->engine()->toScriptValue(m_private->canvas());
  QJSValue cancelAnimationFrameFunction =
      canvasValue.property("cancelAnimationFrameFunction");
  Q_ASSERT(cancelAnimationFrameFunction.isCallable());
  return cancelAnimationFrameFunction.callWithInstance(
      canvasValue, QList<QJSValue>{QJSValue(id)});
}

QJSValue LottiePrivateWindow::lottie() const {
  return m_private->lottieInstance();
}

void LottiePrivateWindow::setLottie(QJSValue lottie) {
  m_private->setLottieInstance(lottie);
  emit lottieChanged();
}

void LottiePrivateWindow::suspend() {
  for (QMap<int, TimerData>::iterator i = m_timers.begin(); i != m_timers.end();
       ++i) {
    TimerData& td = i.value();
    if (td.m_timer) {
      td.m_remainingInterval = td.m_timer->remainingTime();
      td.m_timer->stop();
    }
  }
}

void LottiePrivateWindow::resume() {
  for (QMap<int, TimerData>::iterator i = m_timers.begin(); i != m_timers.end();
       ++i) {
    TimerData& td = i.value();
    if (td.m_remainingInterval >= 0 && td.m_timer) {
      td.m_timer->start(td.m_remainingInterval);
      td.m_remainingInterval = 0;
    }
  }
}
