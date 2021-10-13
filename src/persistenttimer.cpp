#include "persistenttimer.h"
#include "settingsholder.h"
#include "logger.h"

#include <QTimer>
#include <QMap>

namespace {
Logger logger(LOG_MAIN, "PersistentTimer");
}  // namespace

PersistentTimer::PersistentTimer(QObject* parent, const QString& timerName,
                                 int interval_msec)
    : QObject(parent), m_name(timerName), m_interval(interval_msec) {
  m_timer.setInterval(interval_msec);
  connect(&m_timer, &QTimer::timeout, this, &PersistentTimer::timeoutInternal);
}

void PersistentTimer::timeoutInternal() {
  logger.info() << "Timer(" << m_name << ") - Fired!";

  QMap<QString, QVariant> state =
      SettingsHolder::instance()->persistentTimerState();
  QDateTime now = QDateTime::currentDateTime();
  state.insert(m_name, now);
  SettingsHolder::instance()->setPersistentTimerState(state);
  emit timeout();
  m_timer.setInterval(m_interval);
  m_timer.start();
}

void PersistentTimer::start() {
  logger.info() << "Creating Long Timer: " << m_name
                << " every: " << (m_interval / 1000) << "s";
  QMap<QString, QVariant> state =
      SettingsHolder::instance()->persistentTimerState();
  QVariant timerState = state.value(m_name);

  if (!timerState.isValid() || timerState.isNull()) {
    // This Timer has never ever started or completed.
    // So fire it now and then at the next Interval.
    logger.info() << "Timer(" << m_name << ") - No state yet - fire";

    return timeoutInternal();
  }
  if (!timerState.canConvert(QVariant::DateTime)) {
    // Wierd.
    logger.error() << "Timer(" << m_name << ") -  Malformed state - fire";
    return timeoutInternal();
  }
  QDateTime lastRun = timerState.toDateTime();
  QDateTime now = QDateTime::currentDateTime();
  auto time_since_last_run = (lastRun.secsTo(now));
  auto required_secs = (m_interval / 1000) - time_since_last_run;
  if (time_since_last_run >= (m_interval / 1000)) {
    // We're Overdue!
    logger.info() << "Timer(" << m_name << ") -  s overdue" << required_secs;
    return timeoutInternal();
  }
  // Not overdue, scheudle remaining time.
  logger.info() << "Timer(" << m_name
                << ") -  not overdue will fire in: " << required_secs;
  m_timer.start(required_secs * 1000);
}
