/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATUSICON_H
#define STATUSICON_H

#include <QIcon>
#include <QObject>
#include <QTimer>

class StatusIcon final : public QObject
{
    Q_OBJECT

public:
    StatusIcon();

signals:
    void iconChanged(const QIcon &icon);

public slots:
    void controllerStateChanged();

private slots:
    void animateIcon();

private:
    void activateAnimation();

    void setIcon(const QIcon &icon);

private:
    // Animated icon.
    QTimer m_animatedIconTimer;
    uint8_t m_animatedIconIndex = 0;
};

#endif // STATUSICON_H
