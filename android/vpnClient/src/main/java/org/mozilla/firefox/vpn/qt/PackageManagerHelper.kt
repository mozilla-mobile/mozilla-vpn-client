/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
package org.mozilla.firefox.vpn.qt

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.graphics.drawable.Drawable
import android.net.Uri
import org.json.JSONObject
import java.util.ArrayList
import java.util.Arrays

/**
 * PackageManagerHelper offers JNI bindings to 
 * query the Installed apps and fetch their icons
 * it's used in 
 * /platforms/android/androidapplistprovider.cpp
 */
object PackageManagerHelper {
    const val TAG = "PackageManagerHelper"

    @SuppressLint("QueryPermissionsNeeded") // We have QUERY_ALL_PACKAGES permission
    @JvmStatic
    fun getAllAppNames(ctx: Context): JsonAppMap {
        val output = JSONObject()
        val pm = ctx.packageManager
        val browsers = getBrowserIDs(pm)
        val installedPackages = pm.getInstalledPackages(PackageManager.GET_PERMISSIONS)
        installedPackages.stream().filter {
            if (isSelf(it)) {
                // Cannot exclude ourselves
                return@filter false
            }
            // If it's allowlisted, just keep it
            if (isAllowListed(it.packageName)) {
                return@filter true
            }
            // Make sure we add all system web-browsers
            if (browsers.contains(it.packageName)) {
                return@filter true
            }
            // Otherwise just add any app that is not
            // a system package.
            return@filter !isSystemPackage(it, pm)
        }.forEach {
            output.put(it.packageName, it.applicationInfo.loadLabel(pm).toString())
        }
        return output.toString()
    }

    @JvmStatic
    fun getAppIcon(ctx: Context, id: ApplicationId): Drawable {
        try {
            return ctx.packageManager.getApplicationIcon(id)
        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
        }
        return ColorDrawable(Color.TRANSPARENT)
    }


    // A package is considered a system package if it meets any of the following conditions:  
    // 1. It is not flagged as a system app.  
    // 2. It does not use the Internet.  
    // 3. It has not had any updates.  
    // 4. It has no launchable activity (i.e., it's likely a system service).  
    private fun isSystemPackage(pkgInfo: PackageInfo, pm: PackageManager): Boolean {
        if (pkgInfo.applicationInfo.flags and ApplicationInfo.FLAG_SYSTEM == 0) {
            // no system app
            return false
        }
        // For Systems Packages there are Cases where we want to add it anyway:
        // Has the use Internet permission (otherwise makes no sense)
        // Had at least 1 update (this means it's probably on any AppStore)
        // Has a a launch activity (has a ui and is not just a system service)
        if (!usesInternet(pkgInfo)) {
            return true
        }
        if (!hadUpdate(pkgInfo)) {
            return true
        }
        return pm.getLaunchIntentForPackage(pkgInfo.packageName) == null
    }

    private val ALLOWLISTED_APPS = arrayOf(
        "com.google.android.projection.gearhead",
        "com.google.android.apps.tycho",
    )

    internal fun isAllowListed(packageName: ApplicationId): Boolean {
        return Arrays.stream(ALLOWLISTED_APPS).anyMatch { a: String -> a == packageName }
    }

    internal fun isSelf(pkgInfo: PackageInfo): Boolean {
        return pkgInfo.packageName == "org.mozilla.firefox.vpn" || pkgInfo.packageName == "org.mozilla.firefox.vpn.debug"
    }

    internal fun usesInternet(pkgInfo: PackageInfo): Boolean {
        if (pkgInfo.requestedPermissions == null) {
            return false
        }

        var index = 0
        for (permission in pkgInfo.requestedPermissions) {
            val flag = pkgInfo.requestedPermissionsFlags[index]
            index = +1
            if (Manifest.permission.INTERNET == permission &&
                flag == PackageManager.PERMISSION_GRANTED
            ) {
                return true
            }
        }
        return false
    }

    private fun hadUpdate(pkgInfo: PackageInfo): Boolean {
        return pkgInfo.lastUpdateTime > pkgInfo.firstInstallTime
    }

    // Returns List of all Packages that can classify themselves as browsers
    @SuppressLint("QueryPermissionsNeeded") // We have permission :)
    internal fun getBrowserIDs(pm: PackageManager): List<String> {
        val intent = Intent(Intent.ACTION_VIEW, Uri.parse("https://www.mozilla.org/"))
        intent.addCategory(Intent.CATEGORY_BROWSABLE)
        // We've tried using PackageManager.MATCH_DEFAULT_ONLY flag and found that browsers that
        // are not set as the default browser won't be matched even if they had CATEGORY_DEFAULT set
        // in the intent filter
        val resolveInfos = pm.queryIntentActivities(intent, PackageManager.MATCH_ALL)
        val browsers: MutableList<String> = ArrayList()
        for (info in resolveInfos) {
            val browserID = info.activityInfo.packageName
            browsers.add(browserID)
        }
        return browsers
    }
}

/**
 * A String Containing a an android
 * package identifier.
 */
typealias ApplicationId = String

/**
 * A JSON-String representing
 * A Map<String,String>
 *     with key -> the ApplicationId of the app
 *     with value -> the displayName of the app.
 */
typealias JsonAppMap = String
