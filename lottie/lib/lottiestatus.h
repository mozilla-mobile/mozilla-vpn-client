/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOTTIESTATUS_H
#define LOTTIESTATUS_H

#include <QObject>

class LottieStatus final : public QObject {
  Q_OBJECT

 public:
  Q_PROPERTY(bool playing MEMBER m_playing NOTIFY changed);
  Q_PROPERTY(double currentTime MEMBER m_currentTime NOTIFY changed);
  Q_PROPERTY(int totalTime MEMBER m_totalTime NOTIFY changed);
  Q_PROPERTY(bool error MEMBER m_error NOTIFY changed);
  Q_PROPERTY(QString errorString MEMBER m_errorString NOTIFY changed);

  LottieStatus() = default;
  ~LottieStatus() = default;

  bool playing() const { return m_playing; }

  void update(bool playing, double currentTime, int totalTime) {
    m_playing = playing;
    m_currentTime = currentTime;
    m_totalTime = totalTime;
    m_error = false;
    m_errorString.clear();
  }

  void reset() { update(false, 0, 0); }

  void updateAndNotify(bool playing, double currentTime, int totalTime) {
    update(playing, currentTime, totalTime);
    emit changed(m_playing, m_currentTime, m_totalTime, m_error, m_errorString);
  }

  void updateAndNotify(bool playing) {
    updateAndNotify(playing, m_currentTime, m_totalTime);
  }

  void resetAndNotify() { updateAndNotify(false, 0, 0); }

  void error(const QString& errorString) {
    m_playing = false;
    m_currentTime = 0;
    m_totalTime = 0;
    m_error = true;
    m_errorString = errorString;
    emit changed(m_playing, m_currentTime, m_totalTime, m_error, m_errorString);
  }

 signals:
  void changed(bool playing, double currentTime, int totalTime, bool error,
               const QString& errorString);

 private:
  bool m_playing = false;
  double m_currentTime = 0;
  int m_totalTime = 0;
  bool m_error = false;
  QString m_errorString;
};

#endif  // LOTTIESTATUS_H
