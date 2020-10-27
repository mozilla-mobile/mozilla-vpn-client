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
#include <QtAndroid>
#include <QDebug>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QAndroidIntent>
#include <QAndroidServiceConnection>
#include <QAndroidBinder>
#include <QAndroidParcel>
#include <android/log.h>
#include <QJsonObject>
#include <QJsonDocument>
#include "device.h"
#include "keys.h"
#include "androidjniutils.h"
#include "captiveportal/captiveportal.h"


#define TAG "ANDROID_CONTROLLER"
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,    TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,       TAG, __VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,      TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,       TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,      TAG, __VA_ARGS__)


// Binder Codes for VPNServiceBinder
// See also - VPNServiceBinder.kt
// Actions that are Requestable
const int ACTION_ACTIVATE = 1;
const int ACTION_DEACTIVATE =2;
const int ACTION_REGISTERLISTENER = 3;
// Event Types that will be Dispatched after registration
const int EVENT_CONNECTED = 1;
const int EVENT_DISCONNECTED =2;
const int EVENT_LOGS = 3;
const int EVENT_TXUPDATE =4;





void AndroidController::initialize(const Device *device, const Keys *keys)
    {
        Q_UNUSED(device);
        Q_UNUSED(keys);
        androidJNIUtils::init(); // Inject our Native Implemented Methods into the JNI
        // Start the VPN Service (if not yet) and Bind to it
        QtAndroid::bindService(QAndroidIntent(QtAndroid::androidActivity(), "com.mozilla.vpn.VPNService"),
                                           *this, QtAndroid::BindFlag::AutoCreate);
        m_binder.setController(this);
        // TODO: Check the State - might be not OFF on Connection.
        emit initialized(true, Controller::StateOff, QDateTime());
        LOGD("ANDROID-CONTROLLER -- INITIALISED");
    }

void AndroidController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               const CaptivePortal &captivePortal,
                               bool forSwitching)
{
    Q_UNUSED(captivePortal);

    // Serialise arguments for the VPNService
    QJsonObject jDevice;
    jDevice["publicKey"] = device->publicKey();
    jDevice["name"] = device->name();
    jDevice["createdAt"] = device->createdAt().toMSecsSinceEpoch();
    jDevice["ipv4Address"] = device->ipv4Address();
    jDevice["ipv6Address"] = device->ipv6Address();

    QJsonObject jKeys;
    jKeys["privateKey"] = keys->privateKey();

    QJsonObject jServer;
    jServer["ipv4AddrIn"]  = server.ipv4AddrIn();
    jServer["ipv4Gateway"] = server.ipv4Gateway();
    jServer["ipv6AddrIn"]  = server.ipv6AddrIn();
    jServer["ipv6Gateway"] = server.ipv6Gateway();
    jServer["publicKey"] = server.publicKey();
    jServer["port"] = (int) server.choosePort();

    QJsonObject args;
    args["device"] = jDevice;
    args["keys"] = jKeys;
    args["server"] = jServer;
    args["forSwitching"] = forSwitching;

    QJsonDocument doc;
    doc.setObject(args);

    QAndroidParcel sendData;
    sendData.writeData(doc.toJson());
    LOGD("ANDROID-CONTROLLER --  Send Activation Request to Service");
    
    m_serviceBinder.transact(ACTION_ACTIVATE,sendData, nullptr);
}

void AndroidController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);
    QAndroidParcel nullData;
    m_serviceBinder.transact(ACTION_DEACTIVATE,nullData, nullptr);
}

void AndroidController::checkStatus()
{
    // TODO: Implement Really.
    m_txBytes += QRandomGenerator::global()->generate() % 100000;
    m_rxBytes += QRandomGenerator::global()->generate() % 100000;

    emit statusUpdated("127.0.0.1", m_txBytes, m_rxBytes);
}

void AndroidController::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);
    callback("ANDROID-CONTROLLER --  DummyController is always happy");
}

void AndroidController::onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder){
    Q_UNUSED(name);
    m_serviceBinder = serviceBinder;

    // Send the Service our Binder to recive incoming Events
    QAndroidParcel binderParcel;
    binderParcel.writeBinder(m_binder);
    m_serviceBinder.transact(ACTION_REGISTERLISTENER,binderParcel,nullptr);

    LOGD("ANDROID-CONTROLLER -- The VPN service is Connected to the QT main thread");

}

void AndroidController::onServiceDisconnected(const QString &name){
    Q_UNUSED(name);
    // TODO: Maybe restart? Or crash?
    LOGD("ANDROID-CONTROLLER -- The VPN service has disconnected from the Activity ");
}




/**
 * @brief AndroidController::VPNBinder::onTransact
 * @param code the Event-Type we get From the VPNService See
 * @param data - Might contain UTF-8 JSON in case the Event has a payload
 * @param reply - always null
 * @param flags - unused
 * @return Returns true is the code was a valid Event Code
 */
bool AndroidController::VPNBinder::onTransact(int code, const QAndroidParcel &data, const QAndroidParcel &reply, QAndroidBinder::CallType flags){
   Q_UNUSED(data);
   Q_UNUSED(reply);
   Q_UNUSED(flags);
   LOGD("ANDROID-VPNBINDER -- Recived Event %i ",code);
    switch (code) {
    case EVENT_CONNECTED:
        emit mController->connected();
        break;
    case EVENT_DISCONNECTED:
       emit mController->disconnected();
        break;
    case EVENT_TXUPDATE:
    case EVENT_LOGS:
        // TODO: Implement.
        break;
    default:
        return false;
    }
    return true;
}

void AndroidController::VPNBinder::setController(AndroidController* c){
   mController = c;
}
