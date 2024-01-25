/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt

import android.Manifest
import android.app.Activity
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import androidx.test.core.content.pm.ApplicationInfoBuilder
import androidx.test.core.content.pm.PackageInfoBuilder
import android.content.pm.PackageManager.PERMISSION_GRANTED
import android.content.pm.PackageManager.PERMISSION_DENIED
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.Robolectric
import org.robolectric.RobolectricTestRunner
import org.robolectric.Shadows.shadowOf

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
@RunWith(RobolectricTestRunner::class)
class PackageManagerHelperTests {
    @Test
    fun addition_isCorrect() {
        assertEquals(4, 2 + 2)
    }

    /**
     * PackageManagerHelper.getAllAppNames(ctx)
     * Should return the default robolectric app list
     */
    @Test
    fun getAllAppNames_returns_a_list() {
        val ctx: Context = Robolectric.buildActivity(Activity::class.java).create().get()
        val apps = PackageManagerHelper.getAllAppNames(ctx)
        assertEquals("{\"org.mozilla.firefox.vpn.qt.test\":\"org.mozilla.firefox.vpn.qt.test\"}", apps)
    }

    /**
     * Should return only true if it's the correct mozilla vpn uri.
     */
    @Test
    fun is_self() {
        data class TestCase(val name: String, val result: Boolean)
        for (testCase in arrayOf(
            TestCase("org.mozilla.firefox.vpn", true),
            TestCase("org.mozilla.firefox.vpn.debug", true),
            TestCase("org.mozilla.firefox.vpn.somethingElse", false),
            TestCase("org.mozilla.firefox", false),
        )) {
            val pkg = PackageInfoBuilder.newBuilder().setPackageName(testCase.name).build()
            assertEquals(testCase.result, PackageManagerHelper.isSelf(pkg))
        }
    }

    /**
     * usesInternet should reflect the permission of an application
     */
    @Test
    fun usesInternet_checksPermissions() {
        // Should be false, as no internet permission is requested.
        val notFirefox = PackageInfoBuilder.newBuilder()
            .setPackageName("org.mozilla.not.a.browser")
            .setApplicationInfo(
                ApplicationInfoBuilder
                    .newBuilder()
                    .setPackageName("org.mozilla.not.a.browser")
                    .setName("OfflineFox").build(),
            ).build()
        assertEquals(false, PackageManagerHelper.usesInternet(notFirefox))

        // Should be true for a normal internet using apk
        val firefox = PackageInfoBuilder.newBuilder()
            .addRequestedPermission(Manifest.permission.INTERNET, PERMISSION_GRANTED)
            .setPackageName("org.mozilla.firefox")
            .setApplicationInfo(
                ApplicationInfoBuilder
                    .newBuilder()
                    .setPackageName("org.mozilla.firefox")
                    .setName("firefox").build(),
            ).build()
        assertEquals(true, PackageManagerHelper.usesInternet(firefox))
        // Should be false, if the permission was denied / removed somehow.
        val illegalFirefox = PackageInfoBuilder.newBuilder()
            .addRequestedPermission(Manifest.permission.INTERNET, PERMISSION_DENIED)
            .setPackageName("org.mozilla.illegalfox")
            .setApplicationInfo(
                ApplicationInfoBuilder
                    .newBuilder()
                    .setPackageName("org.mozilla.illegalfox")
                    .setName("illegalfox").build(),
            ).build()
        assertEquals(false, PackageManagerHelper.usesInternet(illegalFirefox))
    }

    @Test
    fun getBrowserIDs_queriesIntentFilters() {
        val ctx: Context = Robolectric.buildActivity(Activity::class.java).create().get()
        val pm = shadowOf(ctx.packageManager)

        // Install a fake Application
        val firefox = PackageInfoBuilder.newBuilder()
            .addRequestedPermission(Manifest.permission.INTERNET, PERMISSION_GRANTED)
            .setPackageName("org.mozilla.firefox")
            .setApplicationInfo(
                ApplicationInfoBuilder
                    .newBuilder()
                    .setPackageName("org.mozilla.firefox")
                    .setName("firefox").build(),
            ).build()
        pm.installPackage(firefox)
        // We expect 0 as a Internet Using Package != Browser.
        assertEquals(0, PackageManagerHelper.getBrowserIDs(ctx.packageManager).size)
        // Create an intent filter so that android knows "firefox"
        // can handle http requests
        val fakeFilter = IntentFilter(Intent.ACTION_VIEW)
        fakeFilter.addCategory(Intent.CATEGORY_BROWSABLE)
        fakeFilter.addDataScheme("https")
        fakeFilter.addDataScheme("http")
        val component = ComponentName("org.mozilla.firefox", "FakeActivity")
        pm.addActivityIfNotPresent(component)
        pm.addIntentFilterForActivity(component, fakeFilter)
        // Therefore now it should register one browser installed.
        val result = PackageManagerHelper.getBrowserIDs(ctx.packageManager)
        assertEquals(1, result.size)
        assertEquals("org.mozilla.firefox", result[0])
    }

    @Test fun isAllowListed_working() {
        data class TestCase(val name: String, val result: Boolean)
        for (testCase in arrayOf(
            TestCase("com.google.android.projection.gearhead", true),
            TestCase("org.lol", false),
            TestCase("", false),
        )) {
            assertEquals(testCase.result, PackageManagerHelper.isAllowListed(testCase.name))
        }
    }
}
