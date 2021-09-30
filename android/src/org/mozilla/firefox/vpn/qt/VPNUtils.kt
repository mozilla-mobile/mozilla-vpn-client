/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@file:Suppress("unused")

package org.mozilla.firefox.vpn.qt

import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.os.Build
import android.provider.Settings

// Companion for AndroidUtils.cpp
object VPNUtils {
    @SuppressLint("NewApi")
    @JvmStatic
    fun getDeviceID(ctx: Context):String{
        return Settings.Secure.getString(ctx.contentResolver, Settings.Secure.ANDROID_ID);
    }
}