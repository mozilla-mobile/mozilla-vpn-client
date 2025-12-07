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
import android.util.Log
import androidx.annotation.RequiresApi

/**
 * Helper class for managing battery optimization settings.
 * 
 * Battery optimization can cause Android to kill VPN services after a few hours
 * or prevent them from starting after device reboot. This helper provides methods
 * to detect battery optimization status and guide users to disable it.
 */
object BatteryOptimizationHelper {
    private const val TAG = "BatteryOptimizationHelper"

    /**
     * Check if battery optimizations are disabled for this app.
     * 
     * @param context Application context
     * @return true if optimizations are disabled (good for VPN), false if enabled
     */
    @JvmStatic
    fun isIgnoringBatteryOptimizations(context: Context): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            // Battery optimization was introduced in Android M (API 23)
            return true
        }

        return try {
            val powerManager = context.getSystemService(Context.POWER_SERVICE) as PowerManager
            powerManager.isIgnoringBatteryOptimizations(context.packageName)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to check battery optimization status", e)
            // Assume it's okay if we can't check (fail-safe)
            true
        }
    }

    /**
     * Check if we have permission to request battery optimization exemption.
     * 
     * @param context Application context
     * @return true if permission is granted
     */
    @JvmStatic
    fun hasRequestIgnoreBatteryOptimizationsPermission(context: Context): Boolean {
        return try {
            val permission = "android.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS"
            context.packageManager.checkPermission(permission, context.packageName) == 
                PackageManager.PERMISSION_GRANTED
        } catch (e: Exception) {
            Log.e(TAG, "Failed to check REQUEST_IGNORE_BATTERY_OPTIMIZATIONS permission", e)
            false
        }
    }

    /**
     * Get intent to request battery optimization exemption or open settings.
     * 
     * This method returns different intents based on:
     * - Whether we have REQUEST_IGNORE_BATTERY_OPTIMIZATIONS permission
     * - Android version
     * 
     * @param context Application context
     * @return Intent to open appropriate settings screen, or null if failed
     */
    @RequiresApi(Build.VERSION_CODES.M)
    @JvmStatic
    fun getRequestIgnoreBatteryOptimizationsIntent(context: Context): Intent? {
        return try {
            if (hasRequestIgnoreBatteryOptimizationsPermission(context)) {
                // Can request directly via system dialog
                val intent = Intent(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS)
                intent.data = Uri.parse("package:${context.packageName}")
                intent
            } else {
                // Need to guide user to settings manually
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    // Android 12+: Go to app details
                    Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                        data = Uri.parse("package:${context.packageName}")
                    }
                } else {
                    // Android 6-11: Go to battery optimization list
                    Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS)
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to create battery optimization intent", e)
            null
        }
    }

    /**
     * Check if background restrictions are enabled for this app.
     * Background restrictions prevent apps from running in the background.
     * 
     * @param context Application context
     * @return true if restrictions are enabled (bad for VPN), false otherwise
     */
    @RequiresApi(Build.VERSION_CODES.P)
    fun isBackgroundRestricted(context: Context): Boolean {
        return try {
            val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as android.app.ActivityManager
            activityManager.isBackgroundRestricted
        } catch (e: Exception) {
            Log.e(TAG, "Failed to check background restrictions", e)
            false
        }
    }

    /**
     * Check if background data is restricted for this app.
     * Background data restrictions prevent apps from using data in the background.
     * 
     * @param context Application context
     * @return true if data is restricted (bad for VPN), false otherwise
     */
    @RequiresApi(Build.VERSION_CODES.N)
    fun isBackgroundDataRestricted(context: Context): Boolean {
        return try {
            val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as android.net.ConnectivityManager
            connectivityManager.restrictBackgroundStatus == android.net.ConnectivityManager.RESTRICT_BACKGROUND_STATUS_ENABLED
        } catch (e: Exception) {
            Log.e(TAG, "Failed to check background data restrictions", e)
            false
        }
    }

    /**
     * Get a user-friendly description of why battery optimization should be disabled.
     * 
     * @return String explaining the issue
     */
    @JvmStatic
    fun getBatteryOptimizationExplanation(): String {
        return "Mozilla VPN may disconnect after a few hours and won't restart after device reboot " +
               "because battery optimization is enabled.\n\n" +
               "For reliable VPN operation, please disable battery optimization for Mozilla VPN.\n\n" +
               "This allows the VPN to maintain your secure connection continuously."
    }

    /**
     * Get a short warning message for battery optimization.
     * 
     * @return Short warning string
     */
    fun getBatteryOptimizationWarning(): String {
        return "⚠️ Battery optimization is enabled - VPN may disconnect"
    }

    /**
     * Log battery optimization status and related restrictions.
     * Useful for troubleshooting.
     * 
     * @param context Application context
     * @param tag Log tag to use
     */
    fun logBatteryOptimizationStatus(context: Context, tag: String = TAG) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            val isIgnoring = isIgnoringBatteryOptimizations(context)
            if (isIgnoring) {
                Log.i(tag, "✓ Battery optimization is disabled - VPN should remain stable")
            } else {
                Log.w(tag, "⚠️ Battery optimization is ENABLED - VPN may disconnect after a few hours!")
                Log.w(tag, "⚠️ User should disable battery optimization for reliable VPN operation")
            }

            // Check background restrictions on Android P+
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                if (isBackgroundRestricted(context)) {
                    Log.w(tag, "⚠️ Background restrictions are enabled - VPN may not work properly!")
                }
            }

            // Check background data restrictions on Android N+
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                if (isBackgroundDataRestricted(context)) {
                    Log.w(tag, "⚠️ Background data is restricted - VPN may not work properly!")
                }
            }
        } else {
            Log.i(tag, "Battery optimization checks not available on Android < 6.0")
        }
    }
}


