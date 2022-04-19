/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@file:Suppress("unused")

package org.mozilla.firefox.vpn.qt

import android.annotation.SuppressLint
import android.content.ContentValues
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.provider.MediaStore
import android.provider.Settings
import android.util.Log
import org.mozilla.firefox.vpn.glean.GleanEvent
import java.io.IOException
import java.lang.Exception
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

// Companion for AndroidUtils.cpp
object VPNUtils {
    @SuppressLint("NewApi")
    @JvmStatic
    fun getDeviceID(ctx: Context): String {
        return Settings.Secure.getString(ctx.contentResolver, Settings.Secure.ANDROID_ID)
    }

    @SuppressLint("NewApi")
    @JvmStatic
    fun sharePlainText(text: String): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            // Not supported on oldies. :c
            return false
        }
        val ctx: Context = VPNActivity.getInstance()
        val resolver = ctx.contentResolver

        // Find the right volume to use:
        val collection = MediaStore.Downloads.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY)
        val dateTime = LocalDateTime.now().format(DateTimeFormatter.ofPattern("y-mm-dd-H-m-ss"))
        val fileMetaData = ContentValues().apply {
            put(MediaStore.Downloads.MIME_TYPE, "text/plain")
            put(MediaStore.Downloads.DISPLAY_NAME, "MozillaVPN_Logs_$dateTime")
            put(MediaStore.Downloads.IS_PENDING, 1)
        }
        // Create the File and get the URI
        val fileURI: Uri? = resolver.insert(collection, fileMetaData)
        if (fileURI == null) {
            return false
        }

        val tx = resolver.openOutputStream(fileURI)
        if (tx == null) {
            return false
        }
        try {
            val writer = tx.writer(Charsets.UTF_8)
            writer?.write(text)
            writer?.flush()
        } catch (e: IOException) {
            return false
        }
        tx.flush()
        tx.close()
        // Now update the Files meta data that the file exists
        fileMetaData.clear()
        fileMetaData.put(MediaStore.Downloads.IS_PENDING, 0)

        try {
            val ok = resolver.update(fileURI, fileMetaData, null, null)
            if (ok == 0) {
                Log.e("MozillaVPNLogs", "resolver update - err: 0 Rows updated")
            }
        } catch (e: Exception) {
            Log.e("MozillaVPNLogs", "resolver update - exception: " + e.message)
        }

        val sendIntent = Intent(Intent.ACTION_SEND)
        sendIntent.putExtra(Intent.EXTRA_STREAM, fileURI)
        sendIntent.setType("*/*")

        val chooseIntent = Intent.createChooser(sendIntent, "Share Logs")
        ctx.startActivity(chooseIntent)
        return true
    }

    @SuppressLint("NewApi")
    @JvmStatic
    fun openNotificationSettings() {
        val context = VPNActivity.getInstance()
        val intent = Intent()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            intent.setAction(Settings.ACTION_APP_NOTIFICATION_SETTINGS)
            intent.putExtra(Settings.EXTRA_APP_PACKAGE, context.getPackageName())
        } else {
            intent.setAction("android.settings.APP_NOTIFICATION_SETTINGS")
            intent.putExtra("app_package", context.getPackageName())
            intent.putExtra("app_uid", context.getApplicationInfo().uid)
        }
        context.startActivity(intent)
    }

    fun recordGleanEvent(metricName: GleanEvent) {
        // Explicit cast to string so the jni
        // does not explode here :)
        recordGleanEvent(metricName.toString())
    }

    @SuppressLint("Unused")
    @JvmStatic
    private external fun recordGleanEvent(metricName: String)

    @SuppressLint("Unused")
    @JvmStatic
    private external fun getIsDevMode(): Boolean

    val isDevMode by lazy {
        // "by lazy" means, we evaluate this on first
        // get() and never again.
        getIsDevMode()
    }
}
