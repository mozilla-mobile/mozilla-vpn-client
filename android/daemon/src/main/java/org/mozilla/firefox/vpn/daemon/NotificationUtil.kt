/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.annotation.SuppressLint
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Parcel
import androidx.core.app.NotificationCompat
import org.json.JSONObject

class NotificationUtil {
    val NOTIFICATION_CHANNEL_ID = "com.mozilla.vpnNotification"
    val CONNECTED_NOTIFICATION_ID = 1337
    val tag = "NotificationUtil"
    val sCurrentContext: Context
    private val mNotificationBuilder: NotificationCompat.Builder
    private val mNotificationManager: NotificationManager
    private constructor(ctx: Context) {
        sCurrentContext = ctx
        mNotificationManager = ctx.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        mNotificationBuilder = NotificationCompat.Builder(ctx, NOTIFICATION_CHANNEL_ID)
        updateNotificationChannel(null, null) // Will create the channel, will update
    }

    private var mLastMessage = ""
    private var mLastHeader = ""

    companion object {
        var instance: NotificationUtil? = null
        fun get(ctx: Context?): NotificationUtil? {
            if (instance == null) {
                instance = ctx?.let { NotificationUtil(it) }
            }
            return instance
        }
    }

    /**
     * Updates the current shown notification from a
     * Parcel - Gets called from AndroidController.cpp
     */
    fun update(data: Parcel) {
        // [data] is here a json containing the noification content
        val buffer = data.createByteArray()
        val json = buffer?.let { String(it) }
        val content = JSONObject(json)

        update(content.getString("title"), content.getString("message"))
    }

    /**
     * Updates the current shown notification
     */
    private fun update(heading: String, message: String) {
        val notificationManager: NotificationManager =
            sCurrentContext.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        mLastHeader = heading
        mLastMessage = message
        mNotificationBuilder.let {
            it.setContentTitle(heading)
            it.setContentText(message)
            notificationManager.notify(CONNECTED_NOTIFICATION_ID, it.build())
        }
    }

    /**
     * Saves the default translated "connected" notification, in case the vpn gets started
     * without the app.
     */
    fun updateStrings(data: Parcel, context: Context) {
        // [data] is here a json containing the notification content
        val buffer = data.createByteArray()
        val json = buffer?.let { String(it) }
        val content = JSONObject(json)

        val prefs = Prefs.get(context)
        prefs.edit()
            .putString("fallbackNotificationHeader", content.getString("productName"))
            .putString("fallbackNotificationMessage", content.getString("idleText"))
            .apply()

        val channelName = content.getString("notification_group_name")
        val channelDescription = ""
        updateNotificationChannel(channelName, channelDescription)

        Log.v(tag, "Saved new fallback message -> ${content.getString("title")}")
    }

    /*
    * Creates a new Notification using the current set of Strings
    * Shows the notification in the given {context}
    */
    @SuppressLint("NewApi")
    fun show(service: VPNService) {
        // In case we do not have gotten a message to show from the Frontend
        // try to populate the notification with a translated Fallback message
        val prefs = Prefs.get(service)
        val message = mLastMessage.ifEmpty {
            "" + prefs.getString("fallbackNotificationMessage", "Running in the Background")
        }
        val header = mLastHeader.ifEmpty {
            "" + prefs.getString("fallbackNotificationHeader", "Mozilla VPN")
        }

        // Create the Intent that Should be Fired if the User Clicks the notification
        val mainActivityName = "org.mozilla.firefox.vpn.qt.VPNActivity"
        val activity = Class.forName(mainActivityName)
        val intent = Intent(service, activity)
        val pendingIntent = PendingIntent.getActivity(service, 0, intent, 0)
        // Build our notification
        mNotificationBuilder
            .setSmallIcon(R.drawable.icon_mozillavpn_notifiaction)
            .setContentTitle(header)
            .setContentText(message)
            .setOnlyAlertOnce(true)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setContentIntent(pendingIntent)

        service.startForeground(CONNECTED_NOTIFICATION_ID, mNotificationBuilder.build())
    }
    private fun updateNotificationChannel(aTitle: String?, aDescription: String?) {
        // From Oreo on we need to have a "notification channel" to post to.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            return
        }
        val name = aTitle ?: "General"
        val descriptionText = aDescription ?: ""
        val importance = NotificationManager.IMPORTANCE_LOW
        val channel = NotificationChannel(NOTIFICATION_CHANNEL_ID, name, importance).apply {
            description = descriptionText
        }
        // Register the channel with the system
        mNotificationManager.createNotificationChannel(channel)
    }
}
