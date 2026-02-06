/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt

import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import androidx.browser.customtabs.CustomTabsIntent
import androidx.browser.customtabs.CustomTabsService

object CustomTabHelper {
    const val SUCCESS = 0
    const val ERROR_NO_CUSTOM_TABS = 1
    const val ERROR_INVALID_URL = 2
    const val ERROR_LAUNCH_FAILED = 3

    @JvmStatic
    fun openCustomTab(ctx: Context, url: String): Int {
        if (url.isEmpty()) return ERROR_INVALID_URL

        val uri = try {
            Uri.parse(url)
        } catch (e: Exception) {
            return ERROR_INVALID_URL
        }

        if (!isCustomTabsAvailable(ctx)) return ERROR_NO_CUSTOM_TABS

        return try {
            CustomTabsIntent.Builder()
                .setShowTitle(true)
                .setShareState(CustomTabsIntent.SHARE_STATE_OFF)
                .build()
                .launchUrl(ctx, uri)
            SUCCESS
        } catch (e: Exception) {
            ERROR_LAUNCH_FAILED
        }
    }

    @JvmStatic
    fun isCustomTabsAvailable(ctx: Context): Boolean {
        val pm = ctx.packageManager
        val intent = Intent(Intent.ACTION_VIEW, Uri.parse("https://www.mozilla.org"))
        return pm.queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY)
            .any { info ->
                Intent().apply {
                    action = CustomTabsService.ACTION_CUSTOM_TABS_CONNECTION
                    setPackage(info.activityInfo.packageName)
                }.let { pm.resolveService(it, 0) != null }
            }
    }
}
