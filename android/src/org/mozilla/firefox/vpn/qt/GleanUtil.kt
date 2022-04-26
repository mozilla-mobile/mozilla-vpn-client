package org.mozilla.firefox.vpn.qt

import android.util.Log
import mozilla.components.service.glean.BuildInfo
import mozilla.components.service.glean.Glean
import mozilla.components.service.glean.config.Configuration
import mozilla.components.service.glean.private.EventExtras
import mozilla.telemetry.glean.net.HttpURLConnectionUploader
import mozilla.telemetry.glean.private.EventMetricType
import mozilla.telemetry.glean.private.NoExtraKeys
import mozilla.telemetry.glean.private.NoExtras
import org.json.JSONObject
import org.mozilla.firefox.vpn.GleanMetrics.Pings
import org.mozilla.firefox.vpn.GleanMetrics.Sample
import java.lang.UnsupportedOperationException
import java.util.*
import kotlin.reflect.KProperty1

// @MainThread
@Suppress("UNUSED") // All the calls are via jni from the client, so we can ignore
object GleanUtil {
    @JvmStatic
    fun initializeGlean(upload: Boolean = false, channel: String = "staging") {
        val conf = Configuration(HttpURLConnectionUploader(), Configuration.DEFAULT_TELEMETRY_ENDPOINT, channel, null)
        val build = BuildInfo("versionCode", "VersionName", Calendar.getInstance())
        Glean.registerPings(Pings)
        Glean.initialize(
            applicationContext = VPNActivity.getInstance().applicationContext,
            uploadEnabled = upload,
            buildInfo = build,
            configuration = conf
        )
        Log.i("Android-Glean", "Initialised android glean")
    }
    @JvmStatic
    fun setGleanUploadEnabled(upload: Boolean) {
        Glean.setUploadEnabled(upload)
    }
    @JvmStatic
    fun setGleanSourceTag(tags: String = "") {
        // val tag_list = tag.split(",")
        // Glean.setSourceTags(tag_list);
        // TODO: Glean-Android does not support this, yet.
        // We're using this only for functional tests, so no problem i guess c:
        return
    }
    @JvmStatic
    fun sendGleanPings() {
        Pings.main.submit()
    }
    @JvmStatic
    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEvent(sampleName: String) {
        val sample = getSample(sampleName) as? EventMetricType<NoExtraKeys, NoExtras>
        if (sample == null) {
            Log.e("ANDROID-GLEAN", "Ping not found $sampleName")
            return
        }
        sample.record(NoExtras())
    }
    @JvmStatic
    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEventWithExtraKeys(sampleName: String, extraKeysJson: String) {

        // TODO: Currently on any case where we rely on more then 1 Key,
        // creating the extra is complicated. For this one case i guess this is okay.
        // We could do a better work on getting this via reflection, or do more codegen in generate_glean.
        when (sampleName) {
            "onboardingCtaClick" -> {
                val extras = JSONObject(extraKeysJson)
                val extra = Sample.OnboardingCtaClickExtra(
                    panelCta = extras.get("panel_cta").toString(),
                    panelIdx = extras.get("panel_idx").toString(),
                    panelId = extras.get("panel_id").toString()
                )
                Sample.onboardingCtaClick.record(extra)
            }
            else -> {
                val extra = QtExtra(extraKeysJson)
                val sample = getSample(sampleName) as? EventMetricType<*, QtExtra>
                if (sample == null) {
                    Log.e("ANDROID-GLEAN", "Ping not found $sampleName")
                    return
                }
                sample.record(extra)
            }
        }
    }
    @Suppress("UNCHECKED_CAST") // Callers check cast.
    fun getSample(sampleName: String): EventMetricType<*, *> {
        val sampleProperty = Sample.javaClass.kotlin.members.first {
            it.name == sampleName
        } as KProperty1<Sample, EventMetricType<*, *>>
        val sampleInstance = sampleProperty.get(Sample)
        return sampleInstance
    }

    data class QtExtra(val json: String) : EventExtras {
        override fun toFfiExtra(): Pair<IntArray, List<String>> {
            var keys = mutableListOf<Int>()
            var values = mutableListOf<String>()

            val counter = 0
            val extras = JSONObject(json)

            extras.keys().forEach {
                keys.add(counter)
                values.add(extras.get(it).toString())
            }
            if (keys.size > 1) {
                /*
                    When a Sample has more then 1 key, we need to make sure they are passed
                    in the right order to rust. Currently the way we setup the qt-calling, we can't be
                    assured that even if qml is calling the right order, as we serialise to json.
                 */
                throw UnsupportedOperationException("You shall not pass using 2 extra-keys")
            }
            return Pair(IntArray(keys.size, { keys[it] }), values)
        }
    }
}
