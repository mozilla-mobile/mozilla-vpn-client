/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidcontroller.h"
#include "server.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QAndroidJniObject>
#include <QAndroidIntent>
#include <QtAndroid>


void AndroidController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "AndroidController activated" << server.hostname();


    QAndroidJniObject::callStaticMethod<void>(
        "com/mozilla/vpn/VPNService",
        "startQtAndroidService",
        "(Landroid/content/Context;)V",
        QtAndroid::androidActivity().object());

    emit connected();
}

void AndroidController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);

    qDebug() << "AndroidController deactivated";

    //emit disconnected();
}

void AndroidController::checkStatus()
{
    m_txBytes += QRandomGenerator::global()->generate() % 100000;
    m_rxBytes += QRandomGenerator::global()->generate() % 100000;

    emit statusUpdated("127.0.0.1", m_txBytes, m_rxBytes);
}
