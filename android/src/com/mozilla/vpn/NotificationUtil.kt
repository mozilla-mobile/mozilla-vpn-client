/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Parcel
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import org.json.JSONObject
import org.mozilla.firefox.vpn.VPNService

class NotificationUtil private constructor(context: Context) {
    private val sCurrentContext: Context = context

    private var sNotificationBuilder: NotificationCompat.Builder? = null
    private val NOTIFICATION_CHANNEL_ID = "com.mozilla.vpnNotification"
    private val NOTIFICATION_CHANNEL_ID_CAPTIVEPORTAL = "com.mozilla.captivePortalNotification"

    private val CONNECTED_NOTIFICATION_ID = 1337
    private val tag = "NotificationUtil"

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

    // Creates a new Important Notification
    fun notifyCaptivePortal(header: String, message: String) {
        val notificationManager: NotificationManager =
            sCurrentContext?.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        // From Oreo on we need to have a "notification channel" to post to.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "VPN-CaptivePortal"
            val descriptionText = "Notifications about Captive Portal"
            val importance = NotificationManager.IMPORTANCE_HIGH
            val channel = NotificationChannel(NOTIFICATION_CHANNEL_ID_CAPTIVEPORTAL, name, importance).apply {
                description = descriptionText
            }
            // Register the channel with the system
            notificationManager.createNotificationChannel(channel)
        }

        // In case we do not have gotten a message to show from the Frontend
        // try to populate the notification with a translated Fallback message
        // Create the Intent that Should be Fired if the User Clicks the notification
        val mainActivityName = "org.mozilla.firefox.vpn.VPNActivity"
        val activity = Class.forName(mainActivityName)
        val intent = Intent(sCurrentContext, activity)
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP)
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
        intent.putExtra("command", "captive-portal-stop".toByteArray())
        val pendingIntent = PendingIntent.getActivity(sCurrentContext, 1337, intent, PendingIntent.FLAG_CANCEL_CURRENT)
        // Build our notification

        val builder = NotificationCompat.Builder(sCurrentContext!!, NOTIFICATION_CHANNEL_ID_CAPTIVEPORTAL)
        builder.setSmallIcon(org.mozilla.firefox.vpn.R.drawable.ic_mozvpn_round)
            .setContentTitle(header)
            .setContentText(message)
            .setOnlyAlertOnce(true)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setContentIntent(pendingIntent)
            .setStyle(
                NotificationCompat.BigTextStyle()
                    .bigText(message)
            )

        val notification = builder.build()
        NotificationManagerCompat.from(sCurrentContext!!).notify(99, notification)
    }

    /**
     * Updates the current shown notification
     */
    fun update(heading: String, message: String) {
        if (sCurrentContext == null) return
        val notificationManager: NotificationManager =
            sCurrentContext?.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        sNotificationBuilder?.let {
            it.setContentTitle(heading)
                .setContentText(message)
            notificationManager.notify(CONNECTED_NOTIFICATION_ID, it.build())
        }
    }

    /**
     * Saves the default translated "connected" notification, in case the vpn gets started
     * without the app.
     */
    fun saveFallBackMessage(title: String, message: String, context: Context) {
        val prefs =
            context.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE)
        prefs.edit()
            .putString("fallbackNotificationHeader", title)
            .putString("fallbackNotificationMessage", message)
            .apply()
        Log.v(tag, "Saved new fallback message -> $message")
    }

    /*
    * Creates a new Notification using the current set of Strings
    * Shows the notification in the given {context}
    */
    fun show(service: VPNService) {
        sNotificationBuilder = NotificationCompat.Builder(service, NOTIFICATION_CHANNEL_ID)
        val notificationManager: NotificationManager =
            sCurrentContext?.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        // From Oreo on we need to have a "notification channel" to post to.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "vpn"
            val descriptionText = "  "
            val importance = NotificationManager.IMPORTANCE_LOW
            val channel = NotificationChannel(NOTIFICATION_CHANNEL_ID, name, importance).apply {
                description = descriptionText
            }
            // Register the channel with the system
            notificationManager.createNotificationChannel(channel)
        }
        // In case we do not have gotten a message to show from the Frontend
        // try to populate the notification with a translated Fallback message
        val prefs =
            service.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE)
        val message =
            "" + prefs.getString("fallbackNotificationMessage", "Running in the Background")
        val header = "" + prefs.getString("fallbackNotificationHeader", "Mozilla VPN")

        // Create the Intent that Should be Fired if the User Clicks the notification
        val mainActivityName = "org.qtproject.qt5.android.bindings.QtActivity"
        val activity = Class.forName(mainActivityName)
        val intent = Intent(service, activity)
        val pendingIntent = PendingIntent.getActivity(service, 0, intent, 0)
        // Build our notification
        sNotificationBuilder?.let {
            it.setSmallIcon(org.mozilla.firefox.vpn.R.drawable.ic_mozvpn_round)
                .setContentTitle(header)
                .setContentText(message)
                .setOnlyAlertOnce(true)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setContentIntent(pendingIntent)

            service.startForeground(CONNECTED_NOTIFICATION_ID, it.build())
        }
    }

    companion object {
        private var instance: NotificationUtil? = null
        fun get(ctx: Context): NotificationUtil {
            if (instance == null) {
                instance = NotificationUtil(ctx.applicationContext)
            }
            return instance!!
        }
        fun get(): NotificationUtil? {
            return instance
        }
    }
}
