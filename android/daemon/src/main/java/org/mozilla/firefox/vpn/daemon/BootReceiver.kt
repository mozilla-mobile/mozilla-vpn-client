/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import org.mozilla.firefox.qt.common.BatteryOptimizationHelper
import org.mozilla.firefox.qt.common.Prefs

/**
 * Boot receiver that starts the VPN service when the device boots.
 * 
 * IMPORTANT: This receiver may fail to start the VPN service if battery optimization
 * is enabled, especially on Android 12+ (API 31+). Battery optimization should be
 * disabled for reliable VPN auto-start after reboot.
 */
class BootReceiver : BroadcastReceiver() {
    private val TAG = "BootReceiver"

    override fun onReceive(context: Context, arg1: Intent) {
        Log.init(context)
        
        // Check if start on boot is enabled
        if (!Prefs.get(context).getBoolean(START_ON_BOOT, false)) {
            Log.i(TAG, "Start on boot is disabled - exit")
            return
        }
        
        Log.i(TAG, "Device rebooted - attempting to start VPN service")
        
        // Check battery optimization status and log detailed information
        checkBatteryOptimizationStatus(context)
        
        // Attempt to start the VPN service
        val intent = Intent(context, VPNService::class.java)
        intent.putExtra("startOnBoot", true)
        
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                context.startForegroundService(intent)
                Log.i(TAG, "✓ Successfully queued VPN service start (foreground)")
            } else {
                context.startService(intent)
                Log.i(TAG, "✓ Successfully started VPN service")
            }
        } catch (e: IllegalStateException) {
            // This exception commonly happens on Android 12+ when battery optimization is enabled
            Log.e(TAG, "❌ Failed to start VPN service from background!")
            Log.e(TAG, "❌ This is likely because battery optimization is enabled")
            Log.e(TAG, "❌ VPN will not auto-start until user opens the app")
            Log.e(TAG, "Exception: ${e.message}")
            Log.stack(TAG, e.stackTrace)
            
            // Note: We cannot show a notification here reliably on Android 12+
            // The user will need to open the app, which will show the battery optimization warning
        } catch (e: SecurityException) {
            // Permission denied
            Log.e(TAG, "❌ Security exception when starting VPN service")
            Log.e(TAG, "❌ VPN permissions may have been revoked")
            Log.e(TAG, "Exception: ${e.message}")
            Log.stack(TAG, e.stackTrace)
        } catch (e: Exception) {
            // Other unexpected errors
            Log.e(TAG, "❌ Unexpected error starting VPN service: ${e.message}")
            Log.stack(TAG, e.stackTrace)
        }
    }
    
    /**
     * Check and log battery optimization status.
     * This helps diagnose why VPN auto-start may fail after reboot.
     */
    private fun checkBatteryOptimizationStatus(context: Context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            BatteryOptimizationHelper.logBatteryOptimizationStatus(context, TAG)
            
            val isIgnoring = BatteryOptimizationHelper.isIgnoringBatteryOptimizations(context)
            if (!isIgnoring) {
                Log.e(TAG, "⚠️ Battery optimization is enabled - VPN may not start from background!")
                Log.e(TAG, "⚠️ This is especially problematic on Android 12+ (API 31+)")
                Log.e(TAG, "⚠️ User should open the app and disable battery optimization")
                
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    Log.e(TAG, "⚠️ Android 12+ detected - background service start restrictions apply")
                }
            } else {
                Log.i(TAG, "✓ Battery optimization is disabled - VPN should start successfully")
            }
        } else {
            Log.i(TAG, "Battery optimization checks not available on Android < 6.0")
        }
    }
    
    companion object {
        const val START_ON_BOOT = "startOnBoot"
    }
}
