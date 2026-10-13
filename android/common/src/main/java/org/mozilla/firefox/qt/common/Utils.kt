/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.qt.common
import android.app.Activity
import android.content.Context
import android.content.Intent
import java.util.*
import org.mozilla.firefox.qt.common.BuildConfig

// Companion for Utils.cpp
object Utils {

    @JvmStatic
    fun launchPlayStore(activity: Activity) {
        val intent = Intent.makeMainSelectorActivity(
            Intent.ACTION_MAIN,
            Intent.CATEGORY_APP_MARKET,
        )
        activity.startActivity(intent)
    }
}
