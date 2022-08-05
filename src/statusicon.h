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

  Q_PROPERTY(QUrl iconUrl READ iconUrl NOTIFY iconChanged)

 public:
  StatusIcon();
  ~StatusIcon();

  enum EffectiveAppearance {
    EffectiveAppearanceLight,
    EffectiveAppearanceDark,
  };

  QUrl iconUrl() const;

  const QString& iconString() const { return m_iconUrl; }

  const QIcon& icon() const { return m_icon; }

  void setEffectiveAppearance(bool isDarkAppearance);

 signals:
  void iconChanged(const QIcon& icon);

  void effectiveAppearanceChanged();

 public slots:
  void stateChanged();

  void stabilityChanged();

 private slots:
  void animateIcon();

 private:
  void activateAnimation();

  QIcon getIconFromUrl(const QString& iconUrl) const;

  void setIcon(const QString& iconUrl);

 private:
  QString m_iconUrl;

  QIcon m_icon;

  EffectiveAppearance m_effectiveAppearance = EffectiveAppearanceLight;

  // Animated icon.
  QTimer m_animatedIconTimer;
  uint8_t m_animatedIconIndex = 0;
};

#endif  // STATUSICON_H
