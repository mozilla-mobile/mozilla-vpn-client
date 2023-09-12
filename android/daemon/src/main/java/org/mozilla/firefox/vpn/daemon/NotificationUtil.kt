/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.core.app.NotificationCompat
import kotlinx.serialization.Serializable
import org.json.JSONObject
import org.mozilla.firefox.qt.common.Prefs

class NotificationUtil(ctx: Service) {
    private val HAS_ASKED_FOR_PUSH_PERMISSION = "com.mozilla.vpnNotification.didAskForPermission"
    private val NOTIFICATION_CHANNEL_ID = "com.mozilla.vpnNotification"
    private val CONNECTED_NOTIFICATION_ID = 1337
    private val context: Service = ctx
    private val mNotificationBuilder: NotificationCompat.Builder by lazy {
        NotificationCompat.Builder(ctx, NOTIFICATION_CHANNEL_ID)
    }
    private val mNotificationManager: NotificationManager by lazy {
        ctx.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
    }
    private val mainActivityName = "org.mozilla.firefox.vpn.qt.VPNActivity"

    /**
     * Creates a new Notification using the {CannedNotification}
     * Will bring the service into the foreground using that.
     */
    fun show(message: CannedNotification) {
        updateNotificationChannel()
        // Create the Intent that Should be Fired if the User Clicks the notification
        val activity = Class.forName(mainActivityName)
        val intent = Intent(context, activity)
        val pendingIntent =
            PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_IMMUTABLE)
        // Build our notification
        mNotificationBuilder
            .setSmallIcon(R.drawable.icon_mozillavpn_notifiaction)
            .setContentTitle(message.connectedMessage.header)
            .setContentText(message.connectedMessage.body)
            .setOnlyAlertOnce(true)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setContentIntent(pendingIntent)
        context.startForeground(CONNECTED_NOTIFICATION_ID, mNotificationBuilder.build())
    }

    /**
     * Updates the Notification
     * in case there is no notification currently,
     * this is a no-op.
     */
    fun setNotificationText(msg: ClientNotification?) {
        if (msg == null) {
            return
        }
        mNotificationBuilder.let {
            it.setContentTitle(msg.header)
            it.setContentText(msg.body)
            mNotificationManager.notify(CONNECTED_NOTIFICATION_ID, it.build())
        }
    }

    /**
     * Should be called whenever a session is ended.
     * Will upadte the notification to show the Disconnected Message
     */
    fun hide(message: CannedNotification) {
        // Switch the notification to "Disconnected" / or translated version
        // If the VPN-Client is alive, it will override this instantly
        // If not, this fallback is shown.
        setNotificationText(message.disconnectedMessage)
    }

    // Creates / Updates the notification channel we will be using to post
    // the notification to.
    private fun updateNotificationChannel(name: String = "General", descriptionText: String = "") {
        // From Oreo on we need to have a "notification channel" to post to.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            return
        }
        val importance = NotificationManager.IMPORTANCE_LOW
        val channel = NotificationChannel(NOTIFICATION_CHANNEL_ID, name, importance).apply {
            description = descriptionText
        }
        // Register the channel with the system
        mNotificationManager.createNotificationChannel(channel)
    }

    /**
     * Returns true if the permission "android.permission.POST_NOTIFICATIONS"
     * needs to be requested before this class will be functional.
     */
    fun needsNotificationPermission(): Boolean {
        // Android 13
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) {
            Log.i("NotificationUtil", "No need to request permissionf for ${Build.VERSION.SDK_INT}")
            // Below android 13, we will request permissions on
            // implicit due to us being a foreground service
            return false
        }
        val res = context.checkSelfPermission("android.permission.POST_NOTIFICATIONS")
        if (res == PackageManager.PERMISSION_GRANTED) {
            Log.i("NotificationUtil", "Permission Granted")
            // We have the permission, all good
            return false
        }
        // We would need permissions but only ask the user once.
        val prefs = Prefs.get(context)
        if (prefs.getBoolean(HAS_ASKED_FOR_PUSH_PERMISSION, false)) {
            Log.i("NotificationUtil", "We already asked for push permission, not doing again")
            return false
        }
        Log.e("NotificationUtil", "Need to ask for Notificaiton Permission")
        return true
    }

    /**
     * Should be fired once the user has been asked about their notification
     * preference, so we will only ask once.
     */
    fun onNotificationPermissionPromptFired() {
        val prefs = Prefs.get(context)
        prefs.edit().putBoolean(HAS_ASKED_FOR_PUSH_PERMISSION, true).apply()
    }
}

 /*
  * ClientNotification
  * Message sent from the client manually.
  */
@Serializable
data class ClientNotification(val header: String, val body: String)

 /*
  * A "Canned" Notification contains all strings needed for the "(dis-)/connected" flow
  * and is provided by the controller when asking for a connection.
  */
@Serializable
data class CannedNotification(
    // Message to be shown when the Client connects
    val connectedMessage: ClientNotification,
    // Message to be shown when the client disconnects
    val disconnectedMessage: ClientNotification,
    // Product-Name -> Will be used as the Notification Header
    val productName: String,
) {
    companion object {
        /**
         * CannedNotification(json) -> Creates a Canned notification
         * out of a VPN-Client JSON config.
         */
        operator fun invoke(value: JSONObject?): CannedNotification? {
            if (value == null) {
                return null
            }
            val messages = value.getJSONObject("messages")
            return try {
                CannedNotification(
                    ClientNotification(
                        messages.getString("connectedHeader"),
                        messages.getString("connectedBody"),
                    ),
                    ClientNotification(
                        messages.getString("disconnectedHeader"),
                        messages.getString("disconnectedBody"),
                    ),
                    messages.getString("productName"),
                )
            } catch (e: Exception) {
                Log.e("NotificationUtil", "Failed to Parse Notification Object $value")
                null
            }
        }
    }
}
