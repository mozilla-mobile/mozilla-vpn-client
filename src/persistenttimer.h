#ifndef PERSISTENTTIMER_H
#define PERSISTENTTIMER_H

#include <QObject>
#include <QTimer>

/* PersistentTimer is an Alternative for long QTimer tasks.
 * PersistentTimer notes the last Time it fired in SettingsHolder
 * and restores it old state on init.
 * so use it to do stuff like "once every 24h do: xyz"
 */
class PersistentTimer : public QObject {
  Q_OBJECT
 public:
  PersistentTimer(QObject* parent, const QString& timerName, int interval_msec);

  bool isActive() const;
  int remainingTime() const;
  void start();
  void stop();

 signals:
  void timeout();

 private:
  void timeoutInternal();
  QTimer m_timer;
  const QString m_name;
  const int m_interval;
};

#endif  // PERSISTENTTIMER_H
