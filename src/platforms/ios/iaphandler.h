/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QObject>

class QInAppStore;

class IAPHandler final : public QObject
{
    Q_OBJECT

public:
    IAPHandler(QObject *parent) : QObject(parent) {}

    void start(bool restore);

signals:
    void completed();
    void failed();

private:
    void purchaseCompleted();

private:
    QInAppStore* m_appStore = nullptr;
};

#endif // IAPHANDLER_H
