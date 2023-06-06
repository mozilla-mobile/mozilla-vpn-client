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
enum ServiceAction {
  // Activate the vpn. Body requires a json wg-conf
  ACTION_ACTIVATE = 1,
  // Deactivate the vpn. Body is empty
  ACTION_DEACTIVATE = 2,
  // Register an IBinder to recieve events body is an Ibinder
  ACTION_REGISTERLISTENER = 3,
  // Requests an EVENT_STATISTIC_UPDATE to be send
  ACTION_REQUEST_STATISTIC = 4,
  // Requests to clean up the internal log
  ACTION_REQUEST_CLEANUP_LOG = 6,
  // Retry activation using the last config
  // Used when the activation is aborted for VPN-Permission prompt
  ACTION_RESUME_ACTIVATE = 7,
  // Sets the current notification text.
  // Does nothing if there is no notification
  ACTION_SET_NOTIFICATION_TEXT = 8,
  // Sets the fallback text if the OS triggered the VPN-Service
  // to show a notification
  ACTION_SET_NOTIFICATION_FALLBACK = 9,
  // Get's current status
  ACTION_GET_STATUS = 13,
  // Set startOnBoot pref
  ACTION_SET_START_ON_BOOT = 15,
  // Reactivate the last connection (unusued from the client)
  ACTION_REACTIVATE = 16,
  // Clear the VPN storage
  ACTION_CLEAR_STORAGE = 17,
  // Broadcast a change in telemetry preferences
  ACTION_SET_GLEAN_UPLOAD_ENABLED = 18,

};
typedef enum ServiceAction ServiceAction;
// Event Types that will be Dispatched after registration
enum ServiceEvents {
  // The Service has Accecpted our Binder
  // Responds with the current status of the vpn.
  EVENT_INIT = 0,
  // WG-Go has enabled the adapter (empty response)
  EVENT_CONNECTED = 1,
  // WG-Go has disabled the adapter (empty response)
  EVENT_DISCONNECTED = 2,
  // Contains the Current transfered bytes to endpoint x.
  EVENT_STATISTIC_UPDATE = 3,
  // An Error happened during activation
  // Contains the error message
  EVENT_ACTIVATION_ERROR = 5,
  // An event dispatched when the Daemon doesn't have
  // any stored value for the user telemetry preferences.
  //
  // When this event is received, the app should broadcast
  // the telemetry preferences to the daemon ASAP.
  EVENT_REQUEST_GLEAN_UPLOAD_ENABLED = 7,
};
typedef enum ServiceEvents ServiceEvents;

class AndroidVPNActivity : public QObject {
  Q_OBJECT

 public:
  static void maybeInit();
  static AndroidVPNActivity* instance();
  static bool handleBackButton(JNIEnv* env, jobject thiz);
  static void sendToService(ServiceAction type, const QString& data = "");
  static void connectService();

 signals:
  void serviceConnected();
  void serviceDisconnected();
  void eventInitialized(const QString& data);
  void eventConnected(const QString& data);
  void eventDisconnected(const QString& data);
  void eventStatisticUpdate(const QString& data);
  void eventActivationError(const QString& data);
  void eventRequestGleanUploadEnabledState();

 private:
  AndroidVPNActivity();
  void startAtBootChanged();
  void onLogout();

  static void onServiceMessage(JNIEnv* env, jobject thiz, jint messageType,
                               jstring body);
  static void onServiceConnected(JNIEnv* env, jobject thiz);
  static void onServiceDisconnected(JNIEnv* env, jobject thiz);
  void handleServiceMessage(int code, const QString& data);
};

#endif  // ANDROIDVPNACTIVITY_H
