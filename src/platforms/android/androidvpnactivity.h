/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDVPNACTIVITY_H
#define ANDROIDVPNACTIVITY_H

/* This class defines the Natives found in
 * android/../Qt/VpnActivity.java
 */

#include <QObject>
#include "jni.h"

  // Binder Codes for VPNServiceBinder
  // See also - VPNServiceBinder.kt
 // Actions that are Requestable
 enum ServiceAction{
    ACTION_ACTIVATE = 1,
    ACTION_DEACTIVATE = 2,
    ACTION_REGISTERLISTENER = 3,
    ACTION_REQUEST_STATISTIC = 4,
    ACTION_REQUEST_GET_LOG = 5,
    ACTION_REQUEST_CLEANUP_LOG = 6,
    ACTION_RESUME_ACTIVATE = 7,
    ACTION_SET_NOTIFICATION_TEXT = 8,
    ACTION_SET_NOTIFICATION_FALLBACK = 9
  };
  typedef enum ServiceAction ServiceAction;
  // Event Types that will be Dispatched after registration
  enum ServiceEvents{
    EVENT_INIT = 0,
    EVENT_CONNECTED = 1,
    EVENT_DISCONNECTED = 2,
    EVENT_STATISTIC_UPDATE = 3,
    EVENT_BACKEND_LOGS = 4,
    EVENT_ACTIVATION_ERROR = 5,
  };
  typedef enum ServiceEvents ServiceEvents;


class AndroidVPNActivity : public QObject {
  Q_OBJECT


 public:
  static void maybeInit();
  static AndroidVPNActivity* instance();
  static bool handleBackButton(JNIEnv* env, jobject thiz);
  static void sendToService(ServiceAction type, const QString& data);
  static void connectService();


  signals:
    void serviceConnected();
    void serviceDisconnected();
    void serviceInitialized(const QString& data);
    void serviceVPNConnected(const QString& data);
    void serviceVPNDisconnected(const QString& data);
    void serviceStatisticReport(const QString& data);
    void serviceBackendLogs(const QString& data);
    void serviceVPNActivationError(const QString& data);


 private:
  AndroidVPNActivity();

  static void onServiceMessage(JNIEnv* env, jobject thiz, jint messageType, jstring body);
  static void onServiceConnected(JNIEnv* env, jobject thiz);
  static void onServiceDisconnected(JNIEnv* env, jobject thiz);
  void handleServiceMessage(int code, const QString& data);

};

#endif  // ANDROIDVPNACTIVITY_H
