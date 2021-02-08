/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidcontroller.h"
#include "ipaddressrange.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/server.h"
#include "settingsholder.h"

#include <QAndroidBinder>
#include <QAndroidIntent>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QAndroidParcel>
#include <QAndroidServiceConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QTextCodec>
#include <QtAndroid>

// Binder Codes for VPNServiceBinder
// See also - VPNServiceBinder.kt
// Actions that are Requestable
const int ACTION_ACTIVATE = 1;
const int ACTION_DEACTIVATE = 2;
const int ACTION_REGISTERLISTENER = 3;
const int ACTION_REQUEST_STATISTIC = 4;
const int ACTION_REQUEST_GET_LOG = 5;
const int ACTION_REQUEST_CLEANUP_LOG = 6;
const int ACTION_RESUME_ACTIVATE = 7;
const int ACTION_ENABLE_START_ON_BOOT = 8;
const int ACTION_SET_NOTIFICATION_TEXT = 9;
const int ACTION_SET_NOTIFICATION_FALLBACK = 10;

// Event Types that will be Dispatched after registration
const int EVENT_INIT = 0;
const int EVENT_CONNECTED = 1;
const int EVENT_DISCONNECTED = 2;
const int EVENT_STATISTIC_UPDATE = 3;
const int EVENT_BACKEND_LOGS = 4;

namespace {
Logger logger(LOG_ANDROID, "AndroidController");
AndroidController* s_instance = nullptr;

}  // namespace

AndroidController::AndroidController() : m_binder(this) {
  MVPN_COUNT_CTOR(AndroidController);

  Q_ASSERT(!s_instance);
  s_instance = this;
}
AndroidController::~AndroidController() {
  MVPN_COUNT_DTOR(AndroidController);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

AndroidController* AndroidController::instance() { return s_instance; }

void AndroidController::initialize(const Device* device, const Keys* keys) {
  logger.log() << "Initializing";

  Q_UNUSED(device);
  Q_UNUSED(keys);

  // Hook in the native implementation for startActivityForResult into the JNI
  JNINativeMethod methods[]{{"startActivityForResult",
                             "(Landroid/content/Intent;)V",
                             reinterpret_cast<void*>(startActivityForResult)}};
  QAndroidJniObject javaClass("org/mozilla/firefox/vpn/VPNPermissionHelper");
  QAndroidJniEnvironment env;
  jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
  env->RegisterNatives(objectClass, methods,
                       sizeof(methods) / sizeof(methods[0]));
  env->DeleteLocalRef(objectClass);

  auto appContext = QtAndroid::androidActivity().callObjectMethod(
      "getApplicationContext", "()Landroid/content/Context;");

  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/VPNService", "startService",
      "(Landroid/content/Context;)V", appContext.object());

  // Start the VPN Service (if not yet) and Bind to it
  QtAndroid::bindService(
      QAndroidIntent(appContext.object(), "org.mozilla.firefox.vpn.VPNService"),
      *this, QtAndroid::BindFlag::AutoCreate);
}

void AndroidController::enableStartAtBoot(bool enabled) {
  QAndroidParcel data;
  data.writeData(QByteArray(1, enabled));
  m_serviceBinder.transact(ACTION_ENABLE_START_ON_BOOT, data, nullptr);
}

/*
 * Sets the current notification text that is shown
 */
void AndroidController::setNotificationText(const QString& title,
                                            const QString& message,
                                            int timerSec) {
  QJsonObject args;
  args["title"] = title;
  args["message"] = message;
  args["sec"] = timerSec;
  QJsonDocument doc(args);
  QAndroidParcel data;
  data.writeData(doc.toJson());
  m_serviceBinder.transact(ACTION_SET_NOTIFICATION_TEXT, data, nullptr);
}

/*
 * Sets fallback Notification text that should be shown in case the VPN
 * switches into the Connected state without the app open
 * e.g via always-on vpn
 */
void AndroidController::setFallbackConnectedNotification() {
  QJsonObject args;
  args["title"] = qtTrId("vpn.main.productName");
  //% "Ready for you to connect"
  //: Refers to the app - which is currently running the background and waiting
  args["message"] = qtTrId("vpn.android.notification.isIDLE");
  QJsonDocument doc(args);
  QAndroidParcel data;
  data.writeData(doc.toJson());
  m_serviceBinder.transact(ACTION_SET_NOTIFICATION_FALLBACK, data, nullptr);
}

void AndroidController::activate(
    const Server& server, const Device* device, const Keys* keys,
    const QList<IPAddressRange>& allowedIPAddressRanges,
    const QList<QString>& vpnDisabledApps, Reason reason) {
  logger.log() << "Activation";

  logger.log() << "Prompting for VPN permission";
  auto appContext = QtAndroid::androidActivity().callObjectMethod(
      "getApplicationContext", "()Landroid/content/Context;");
  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/VPNPermissionHelper", "startService",
      "(Landroid/content/Context;)V", appContext.object());

  m_server = server;

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
  jServer["ipv4AddrIn"] = server.ipv4AddrIn();
  jServer["ipv4Gateway"] = server.ipv4Gateway();
  jServer["ipv6AddrIn"] = server.ipv6AddrIn();
  jServer["ipv6Gateway"] = server.ipv6Gateway();
  jServer["publicKey"] = server.publicKey();
  jServer["port"] = (int)server.choosePort();

  QJsonArray allowedIPs;
  foreach (auto item, allowedIPAddressRanges) {
    QJsonValue val;
    val = item.toString();
    allowedIPs.append(val);
  }

  QJsonArray excludedApps;
  foreach (auto appID, vpnDisabledApps) {
    excludedApps.append(QJsonValue(appID));
  }

  QJsonObject args;
  args["device"] = jDevice;
  args["keys"] = jKeys;
  args["server"] = jServer;
  args["reason"] = (int)reason;
  args["allowedIPs"] = allowedIPs;
  args["excludedApps"] = excludedApps;

  QJsonDocument doc(args);
  QAndroidParcel sendData;
  sendData.writeData(doc.toJson());
  m_serviceBinder.transact(ACTION_ACTIVATE, sendData, nullptr);
}
// Activates the tunnel that is currently set
// in the VPN Service
void AndroidController::resume_activate() {
  QAndroidParcel nullData;
  m_serviceBinder.transact(ACTION_RESUME_ACTIVATE, nullData, nullptr);
}

void AndroidController::deactivate(Reason reason) {
  logger.log() << "deactivation";

  if (reason != ReasonNone) {
    // Just show that we're disconnected
    // we're doing the actual disconnect once
    // the vpn-service has the new server ready in Action->Activate
    emit disconnected();
    logger.log() << "deactivation skipped for Switching";
    return;
  }

  QAndroidParcel nullData;
  m_serviceBinder.transact(ACTION_DEACTIVATE, nullData, nullptr);
}

void AndroidController::checkStatus() {
  logger.log() << "check status";

  QAndroidParcel nullParcel;
  m_serviceBinder.transact(ACTION_REQUEST_STATISTIC, nullParcel, nullptr);
}

void AndroidController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  logger.log() << "get logs";

  m_logCallback = std::move(a_callback);
  QAndroidParcel nullData, replyData;
  m_serviceBinder.transact(ACTION_REQUEST_GET_LOG, nullData, &replyData);
}

void AndroidController::cleanupBackendLogs() {
  logger.log() << "cleanup logs";

  QAndroidParcel nullParcel;
  m_serviceBinder.transact(ACTION_REQUEST_CLEANUP_LOG, nullParcel, nullptr);
}

void AndroidController::onServiceConnected(
    const QString& name, const QAndroidBinder& serviceBinder) {
  logger.log() << "Server connected";

  Q_UNUSED(name);

  m_serviceBinder = serviceBinder;

  // Send the Service our Binder to recive incoming Events
  QAndroidParcel binderParcel;
  binderParcel.writeBinder(m_binder);
  m_serviceBinder.transact(ACTION_REGISTERLISTENER, binderParcel, nullptr);

  // Sync the StartAtBoot Pref as it might have been changed
  // while the controler was not connected
  enableStartAtBoot(SettingsHolder::instance()->startAtBoot());
}

void AndroidController::onServiceDisconnected(const QString& name) {
  logger.log() << "Server disconnected";
  m_serviceConnected = false;
  Q_UNUSED(name);
  // TODO: Maybe restart? Or crash?
}

/**
 * @brief AndroidController::VPNBinder::onTransact
 * @param code the Event-Type we get From the VPNService See
 * @param data - Might contain UTF-8 JSON in case the Event has a payload
 * @param reply - always null
 * @param flags - unused
 * @return Returns true is the code was a valid Event Code
 */
bool AndroidController::VPNBinder::onTransact(int code,
                                              const QAndroidParcel& data,
                                              const QAndroidParcel& reply,
                                              QAndroidBinder::CallType flags) {
  Q_UNUSED(data);
  Q_UNUSED(reply);
  Q_UNUSED(flags);

  QJsonDocument doc;
  QString buffer;
  switch (code) {
    case EVENT_INIT:
      logger.log() << "Transact: init";
      doc = QJsonDocument::fromJson(data.readData());
      emit m_controller->initialized(
          true, doc.object()["connected"].toBool(),
          QDateTime::fromMSecsSinceEpoch(
              doc.object()["time"].toVariant().toLongLong()));
      // Pass a localised version of the Fallback string for the Notification
      m_controller->setFallbackConnectedNotification();

      break;
    case EVENT_CONNECTED:
      logger.log() << "Transact: connected";
      emit m_controller->connected();
      break;
    case EVENT_DISCONNECTED:
      logger.log() << "Transact: disconnected";
      emit m_controller->disconnected();
      break;
    case EVENT_STATISTIC_UPDATE:
      logger.log() << "Transact:: update";

      // Data is here a JSON String
      doc = QJsonDocument::fromJson(data.readData());
      emit m_controller->statusUpdated(m_controller->m_server.ipv4Gateway(),
                                       doc.object()["totalTX"].toInt(),
                                       doc.object()["totalRX"].toInt());
      break;
    case EVENT_BACKEND_LOGS:
      logger.log() << "Transact: backend logs";

      buffer = readUTF8Parcel(data);
      if (m_controller->m_logCallback) {
        m_controller->m_logCallback(buffer);
      }
      break;
    default:
      logger.log() << "Transact: Invalid!";
      break;
  }

  return true;
}

QString AndroidController::VPNBinder::readUTF8Parcel(QAndroidParcel data) {
  // 106 is the Code for UTF-8
  return QTextCodec::codecForMib(106)->toUnicode(data.readData());
}

const int ACTIVITY_RESULT_OK = 0xffffffff;
/**
 * @brief Starts the Given intent in Context of the QTActivity
 * @param env
 * @param intent
 */
void AndroidController::startActivityForResult(JNIEnv* env, jobject /*thiz*/,
                                               jobject intent) {
  logger.log() << "start activity";
  Q_UNUSED(env);
  QtAndroid::startActivity(intent, 1337,
                           [](int receiverRequestCode, int resultCode,
                              const QAndroidJniObject& data) {
                             // Currently this function just used in
                             // VPNService.kt::checkPersmissions. So the result
                             // we're getting is if the User gave us the
                             // Vpn.bind permission. In case of NO we should
                             // abort.
                             Q_UNUSED(receiverRequestCode);
                             Q_UNUSED(data);

                             AndroidController* controller =
                                 AndroidController::instance();
                             if (!controller) {
                               return;
                             }

                             if (resultCode == ACTIVITY_RESULT_OK) {
                               logger.log() << "VPN PROMPT RESULT - Accepted";
                               controller->resume_activate();
                               return;
                             }
                             // If the request got rejected abort the current
                             // connection.
                             logger.log() << "VPN PROMPT RESULT - Rejected";
                             emit controller->disconnected();
                           });
  return;
}
