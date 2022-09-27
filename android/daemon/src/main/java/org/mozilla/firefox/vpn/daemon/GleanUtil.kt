/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.Context
import mozilla.telemetry.glean.BuildInfo
import mozilla.telemetry.glean.Glean
import mozilla.telemetry.glean.config.Configuration
import mozilla.telemetry.glean.net.HttpURLConnectionUploader
import mozilla.telemetry.glean.private.EventExtras
import mozilla.telemetry.glean.private.EventMetricType
import mozilla.telemetry.glean.private.NoExtraKeys
import mozilla.telemetry.glean.private.NoExtras
import org.json.JSONObject
import org.mozilla.firefox.vpn.daemon.GleanMetrics.Pings
import org.mozilla.firefox.vpn.daemon.GleanMetrics.Sample
import java.util.*
import kotlin.reflect.KParameter
import kotlin.reflect.KProperty1
import kotlin.reflect.full.primaryConstructor

// @MainThread
@Suppress("UNUSED") // All the calls are via jni from the client, so we can ignore
class GleanUtil(aParent: Context) {
    private val mParent: Context = aParent

    fun initializeGlean() {
        val gleanEnabled = Prefs.get(mParent).getBoolean("glean_enabled", false) // Don't send telemetry unless explicitly asked to.
        val channel = if (mParent.packageName.endsWith(".debug")) {
            "staging"
        } else {
            "production"
        }
        val conf = Configuration(Configuration.DEFAULT_TELEMETRY_ENDPOINT, channel, 500, HttpURLConnectionUploader())
        val build = BuildInfo(BuildConfig.VERSIONCODE, BuildConfig.SHORTVERSION, Calendar.getInstance())
        Glean.registerPings(Pings)
        Glean.initialize(
            applicationContext = mParent.applicationContext,
            uploadEnabled = gleanEnabled,
            buildInfo = build,
            configuration = conf
        )
        Log.i("Android-Glean", "Initialised android glean")
    }

    fun setGleanUploadEnabled(upload: Boolean) {
        Prefs.get(mParent).edit().apply {
            putBoolean("glean_enabled", upload)
            apply()
        }
        Glean.setUploadEnabled(upload)
    }

    fun setGleanSourceTag(tags: String? = "") {
        if (tags == null) {
            return
        }
        Glean.setSourceTags(tags.split(",").toSet())
        return
    }

    fun sendGleanMainPing() {
        Pings.main.submit()
    }
    fun recordEvent(event: JSONObject) {
        if (event.has("extras")) {
            recordGleanEventWithExtraKeys(event.get("key").toString(), event.get("extras").toString())
        } else {
            recordGleanEvent(event.get("key").toString())
        }
    }

    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEvent(sampleName: String) {
        val sample = getSample(sampleName) as? EventMetricType<NoExtraKeys, NoExtras>
        if (sample == null) {
            return
        }
        sample.record(NoExtras())
    }
    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEventWithExtraKeys(sampleName: String, extraKeysJson: String) {
        val sample = getSample(sampleName) as? EventMetricType<*, EventExtras>
        val extra = getExtra(sampleName, extraKeysJson)
        if (sample == null) {
            Log.e("ANDROID-GLEAN", "Ping not found $sampleName")
            return
        }
        sample.record(extra)
    }

    companion object {
        @Suppress("UNCHECKED_CAST") // Callers check cast.
        fun getSample(sampleName: String): EventMetricType<*, *> {
            val sampleProperty = Sample.javaClass.kotlin.members.first {
                it.name == sampleName
            } as KProperty1<Sample, EventMetricType<*, *>>
            return sampleProperty.get(Sample)
        }

        @Suppress("UNCHECKED_CAST") // Callers check cast.
        fun getExtra(sampleName: String, extraKeysJson: String): EventExtras {
            val extraJSON = JSONObject(extraKeysJson)
            val extrasName = "${sampleName}Extra"
            // Search Sample for the Dataclass of "someNameExtras"
            val extraDataClass = Sample.javaClass.kotlin.nestedClasses.first() {
                it.simpleName.equals(extrasName, ignoreCase = true)
            }
            // Read the Constructor
            val extraConstructor = extraDataClass.primaryConstructor!!
            val paramsList = extraConstructor.parameters
            val args: MutableMap<KParameter, String> = HashMap<KParameter, String>()
            // Fill out the Constructor arg Map for each provided Extra key
            extraJSON.keys().forEach { key ->
                // QML js uses snake style "panel_cta" while the kotlin codegen
                // uses camel case i.e "panelCta"
                val queryKey = key.replace("_", "").lowercase()

                val param: KParameter? = paramsList.firstOrNull() {
                    it.name.equals(queryKey, ignoreCase = true)
                }
                if (param != null) {
                    args[param] = extraJSON.getString(key)
                }
            }
            return extraConstructor.callBy(args) as EventExtras
        }
    }
}
