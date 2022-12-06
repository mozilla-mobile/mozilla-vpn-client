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
import android.os.Build
import androidx.core.app.NotificationCompat
import kotlinx.serialization.Serializable
import org.json.JSONObject

class NotificationUtil
(ctx: Service) {

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
        val pendingIntent = PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_IMMUTABLE)
        // Build our notification
        mNotificationBuilder
            .setSmallIcon(R.drawable.icon_mozillavpn_notifiaction)
            .setContentTitle(message.productName)
            .setContentText(message.connectedMessage)
            .setOnlyAlertOnce(true)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setContentIntent(pendingIntent)
        context.startForeground(CONNECTED_NOTIFICATION_ID, mNotificationBuilder.build())
    }

    /**
     * Updates the Notification
     * in case there is no notification currently, (the client is not in foreground)
     * this is a no-op.
     */
    fun setNotificationText(msg: ClientNotification?) {
        if (msg == null) {
            return
        }
        update(msg.header, msg.body)
    }

    /**
     * Should be called whenever a session is ended.
     * Will upadte the notification to show the Disconnected Message
     */
    fun hide(message: CannedNotification) {
        // Switch the notification to "Disconnected" / or translated version
        // If the VPN-Client is alive, it will override this instantly
        // If not, this fallback is shown.
        update(message.productName, message.disconnectedMessage)
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
    // Updates the current notification
    private fun update(heading: String, message: String) {
        mNotificationBuilder.let {
            it.setContentTitle(heading)
            it.setContentText(message)
            mNotificationManager.notify(CONNECTED_NOTIFICATION_ID, it.build())
        }
    }

    /*
     * A "Canned" Notification contains all strings needed for the "(dis)-connected" flow
     * and is provided by the controller when asking for a connection
     */
    @Serializable
    data class CannedNotification(
        // Message to be shown when the Client connects
        val connectedMessage: String,
        // Message to be shown when the client disconnects
        val disconnectedMessage: String,
        // Product-Name -> Will be used as the Notification Header
        val productName: String,
    ) {
        companion object {
            /**
             * CannedNotification(json) -> Creates a
             */
            operator fun invoke(value: JSONObject?): CannedNotification? {
                if (value == null) {
                    return null
                }
                val messages = value.getJSONObject("messages")
                return try {
                    CannedNotification(
                        messages.getString("connectedMessage"),
                        messages.getString("disconnectedMessage"),
                        messages.getString("productName"),
                    )
                } catch (e: Exception) {
                    null
                }
            }
        }
    }

    /*
     * ClientNotification
     * Message send from the client manually.
     */
    @Serializable
    data class ClientNotification(val header: String, val body: String)
}
