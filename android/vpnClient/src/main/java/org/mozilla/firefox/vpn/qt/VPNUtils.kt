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
import java.io.IOException
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter
import java.util.Calendar

// Companion for AndroidUtils.cpp
object VPNUtils {
    @SuppressLint("NewApi")
    @JvmStatic
    fun getDeviceID(ctx: Context): String {
        return Settings.Secure.getString(ctx.contentResolver, Settings.Secure.ANDROID_ID)
    }

    @SuppressLint("NewApi")
    @JvmStatic
    fun openNotificationSettings() {
        val context = VPNActivity.getInstance()
        val intent = Intent()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            intent.setAction(Settings.ACTION_VPN_SETTINGS)
            intent.putExtra(Settings.EXTRA_APP_PACKAGE, context.getPackageName())
        } else {
            intent.setAction("android.settings.APP_NOTIFICATION_SETTINGS")
            intent.putExtra("app_package", context.getPackageName())
            intent.putExtra("app_uid", context.getApplicationInfo().uid)
        }
        context.startActivity(intent)
    }
}
