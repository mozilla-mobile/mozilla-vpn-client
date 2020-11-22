/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATUSICON_H
#define STATUSICON_H

#include <QIcon>
#include <QObject>
#include <QTimer>
#include <QUrl>

class StatusIcon final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(StatusIcon)

  Q_PROPERTY(QUrl iconUrl READ iconUrl NOTIFY iconChanged)

 public:
  StatusIcon();
  ~StatusIcon();

  QUrl iconUrl() const;

  const QString& iconString() const { return m_icon; }

 signals:
  void iconChanged(const QString& icon);

 public slots:
  void stateChanged();

 private slots:
  void animateIcon();

 private:
  void activateAnimation();

  void setIcon(const QString& icon);

 private:
  QString m_icon;

  // Animated icon.
  QTimer m_animatedIconTimer;
  uint8_t m_animatedIconIndex = 0;
};

#endif  // STATUSICON_H
