/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.qt.common
import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.util.Log
import mozilla.telemetry.glean.BuildInfo
import mozilla.telemetry.glean.Glean
import mozilla.telemetry.glean.config.Configuration
import java.util.*

// Companion for Utils.cpp
object Utils {
    @SuppressLint("NewApi")
    @JvmStatic
    fun initializeGlean(
        ctx: Context,
        isTelemetryEnabled: Boolean,
        channel: String,
        gleanDebugTag: String,
    ) {
        if (!(gleanDebugTag.isEmpty())) {
            Log.i("VPNUtils", "Setting Glean debug tag.")
            Glean.setDebugViewTag(gleanDebugTag)
        }
        Glean.initialize(
            applicationContext = ctx.applicationContext,
            uploadEnabled = isTelemetryEnabled,
            // GleanBuildInfo can only be generated for application,
            // We are in a library so we have to build it ourselves.
            buildInfo = BuildInfo(
                BuildConfig.VERSIONCODE,
                BuildConfig.SHORTVERSION,
                Calendar.getInstance(),
            ),
            configuration = Configuration(channel = channel),
        )
    }

    @JvmStatic
    fun launchPlayStore(activity: Activity) {
        val intent = Intent.makeMainSelectorActivity(
            Intent.ACTION_MAIN,
            Intent.CATEGORY_APP_MARKET,
        )
        activity.startActivity(intent)
    }
}
