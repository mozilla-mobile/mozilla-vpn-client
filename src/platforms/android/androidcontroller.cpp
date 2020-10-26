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

void AndroidController::initialize(const Device *device, const Keys *keys)
    {
        Q_UNUSED(device);
        Q_UNUSED(keys);
        androidJNIUtils::init();
        LOGD("ANDROID-CONTROLLER -- Binding to Backend Service");
        QtAndroid::bindService(QAndroidIntent(QtAndroid::androidActivity(), "com.mozilla.vpn.VPNService"),
                                           *this, QtAndroid::BindFlag::AutoCreate);
        m_binder.setController(this);

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

    // Serialise arguments for the Background Service
    // TODO: Rework Scheme to minimum required properties
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

    QAndroidParcel sendData, replyData;
    //lolno Send own binder so service can async respond
    //sendData.writeBinder(m_binder);
    sendData.writeData(doc.toJson());
    LOGD("ANDROID-CONTROLLER --  Chars %d", doc.toJson().count());
    LOGD("ANDROID-CONTROLLER --  Send Activation Request to Service");
    m_serviceBinder.transact(1,sendData, &replyData);

    bool connectionUp = replyData.readVariant().toBool();
    if(connectionUp){
        emit connected();
        return;
    }
    emit disconnected();
}

void AndroidController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);

    qDebug() << "ANDROID-CONTROLLER --  deactivated";
    LOGD("ANDROID-CONTROLLER --  deactivated");
    QAndroidParcel sendData, replyData;
    sendData.writeVariant(1);
    m_serviceBinder.transact(2,sendData,&replyData);

    try {
        int reply = replyData.readVariant().toInt();

        if(reply == 1){
            emit disconnected();
            LOGD("ANDROID-CONTROLLER --  activated");
        }else{
             emit connected();
        }
    } catch (QAndroidJniExceptionCleaner e) {
        LOGD("ANDROID-CONTROLLER --  failed to read " );
   }
}

void AndroidController::checkStatus()
{
    m_txBytes += QRandomGenerator::global()->generate() % 100000;
    m_rxBytes += QRandomGenerator::global()->generate() % 100000;

    emit statusUpdated("127.0.0.1", m_txBytes, m_rxBytes);
}
// unused
void AndroidController::onRecviceConnected(){
    emit connected();
}
// unused
void AndroidController::onRecviceDisconnected(){
    emit disconnected();
}

void AndroidController::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);
    callback("ANDROID-CONTROLLER --  DummyController is always happy");
}

void AndroidController::onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder){
    m_serviceBinder = serviceBinder;
    qDebug() << "ANDROID-CONTROLLER -- A service is Connected to the QT main thread" << name;
    LOGD("ANDROID-CONTROLLER -- A service is Connected to the QT main thread");
}
void AndroidController::onServiceDisconnected(const QString &name){
    LOGD("ANDROID-CONTROLLER -- A service is DISCONNECTED ");
    qDebug() << "ANDROID-CONTROLLER -- A service is DISCONNECTED " << name;
}





bool AndroidController::VPNBinder::onTransact(int code, const QAndroidParcel &data, const QAndroidParcel &reply, QAndroidBinder::CallType flags){
    qDebug() << " ~~ client: onTransact " << code << data.readVariant() << int(flags);
    reply.writeData("Yupee");
    return true;
}

void AndroidController::VPNBinder::setController(AndroidController* c){
   mController = c;
}
