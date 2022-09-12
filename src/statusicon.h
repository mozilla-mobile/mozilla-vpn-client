/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATUSICON_H
#define STATUSICON_H

#include "connectionhealth.h"

#include <QIcon>
#include <QObject>
#include <QTimer>
#include <QUrl>

class StatusIcon final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(StatusIcon)

 public:
  StatusIcon();
  ~StatusIcon();

  const QIcon& icon();
  const QString iconString();
  const QColor& indicatorColor();

 signals:
  void iconUpdateNeeded();

 public slots:
  void stateChanged();

 private slots:
  void animateIcon();

 private:
  void activateAnimation();
  QIcon drawStatusIndicator();
  void generateIcon();

 private:
  QIcon m_icon;
  QColor m_indicatorColor;

  // Animated icon.
  QTimer m_animatedIconTimer;
  uint8_t m_animatedIconIndex = 0;
};

#endif  // STATUSICON_H
