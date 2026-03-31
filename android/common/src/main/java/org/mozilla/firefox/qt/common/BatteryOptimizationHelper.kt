/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.qt.common

import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.PowerManager
import android.provider.Settings

object BatteryOptimizationHelper {

    @JvmStatic
    fun isIgnoringBatteryOptimizations(context: Context): Boolean {
        return try {
            val pm = context.getSystemService(Context.POWER_SERVICE) as PowerManager
            pm.isIgnoringBatteryOptimizations(context.packageName)
        } catch (e: Exception) { true }
    }

    @JvmStatic
    fun hasRequestIgnoreBatteryOptimizationsPermission(context: Context): Boolean {
        return try {
            context.packageManager.checkPermission(
                "android.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS",
                context.packageName) == PackageManager.PERMISSION_GRANTED
        } catch (e: Exception) { false }
    }

    @JvmStatic
    fun getRequestIgnoreBatteryOptimizationsIntent(context: Context): Intent? {
        return try {
            if (hasRequestIgnoreBatteryOptimizationsPermission(context)) {
                Intent(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS).apply {
                    data = Uri.parse("package:${context.packageName}")
                }
            } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                    data = Uri.parse("package:${context.packageName}")
                }
            } else {
                Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS)
            }
        } catch (e: Exception) { null }
    }
}
