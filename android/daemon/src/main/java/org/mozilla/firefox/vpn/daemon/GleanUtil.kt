package org.mozilla.firefox.vpn.daemon

import android.content.Context
import android.util.Log
import kotlinx.serialization.Serializable
import mozilla.components.service.glean.BuildInfo
import mozilla.components.service.glean.Glean
import mozilla.components.service.glean.config.Configuration
import mozilla.components.service.glean.private.EventExtras
import mozilla.telemetry.glean.net.HttpURLConnectionUploader
import mozilla.telemetry.glean.private.EventMetricType
import mozilla.telemetry.glean.private.NoExtraKeys
import mozilla.telemetry.glean.private.NoExtras
import org.json.JSONObject
import org.mozilla.firefox.vpn.daemon.GleanMetrics.Pings
import org.mozilla.firefox.vpn.daemon.GleanMetrics.Sample
import java.util.*
import kotlin.reflect.KMutableProperty
import kotlin.reflect.KProperty1
import kotlin.reflect.full.createInstance

// @MainThread
@Suppress("UNUSED") // All the calls are via jni from the client, so we can ignore
class GleanUtil(aParent: Context) {
    private val mParent: Context = aParent;

    fun initializeGlean() {
        val gleanEnabled = Prefs.get(mParent).getBoolean("glean_enabled",false) // Don't send telemetry unless explicitly asked to.
        val channel = if(mParent.packageName.endsWith(".debug"))
        {
                "staging"
        } else{
                "production"
        }
        val conf = Configuration(HttpURLConnectionUploader(), Configuration.DEFAULT_TELEMETRY_ENDPOINT, channel, null)
        val build = BuildInfo("versionCode", "VersionName", Calendar.getInstance())
        Glean.registerPings(Pings)
        Log.e("ANDROID-GLEAN", "initializeGlean on:$gleanEnabled, ch:$channel ")
        Glean.initialize(
            applicationContext = mParent.applicationContext,
            uploadEnabled = gleanEnabled,
            buildInfo = build,
            configuration = conf
        )
        Log.i("Android-Glean", "Initialised android glean")
    }

    fun setGleanUploadEnabled(upload: Boolean) {
        Log.e("ANDROID-GLEAN", "setGleanUploadEnabled  $upload")
        Prefs.get(mParent).edit().apply {
            putBoolean("glean_enabled",upload)
            apply()
        }
        Glean.setUploadEnabled(upload)
    }

    fun setGleanSourceTag(tags: String = "") {
        // val tag_list = tag.split(",")
        // Glean.setSourceTags(tag_list);
        // TODO: Glean-Android does not support this, yet.
        // We're using this only for functional tests, so no problem i guess c:
        return
    }

    fun sendGleanPings() {
        Log.e("ANDROID-GLEAN", "sendGleanPings ")
        Pings.main.submit()
    }
    fun recordEvent(event :JSONObject){
        if(event.has("extras")){
            recordGleanEventWithExtraKeys(event.get("key").toString(),event.get("extras").toString())
        }else{
            recordGleanEvent(event.get("key").toString())
        }
    }

    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEvent(sampleName: String) {
        Log.e("ANDROID-GLEAN", "recordGleanEvent $sampleName")
        val sample = getSample(sampleName) as? EventMetricType<NoExtraKeys, NoExtras>
        if (sample == null) {
            Log.e("ANDROID-GLEAN", "Ping not found $sampleName")
            return
        }
        sample.record(NoExtras())
    }
    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEventWithExtraKeys(sampleName: String, extraKeysJson: String) {
        Log.e("ANDROID-GLEAN", "recordGleanEventWithExtraKeys $sampleName / $extraKeysJson")
        val sample = getSample(sampleName) as? EventMetricType<*, EventExtras>
        val extra = getExtra(sampleName, extraKeysJson)
        if (sample == null) {
            Log.e("ANDROID-GLEAN", "Ping not found $sampleName")
            return
        }
        sample.record(extra)
    }

    @Suppress("UNCHECKED_CAST") // Callers check cast.
    fun getSample(sampleName: String): EventMetricType<*, *> {
        val sampleProperty = Sample.javaClass.kotlin.members.first {
            it.name == sampleName
        } as KProperty1<Sample, EventMetricType<*, *>>
        return sampleProperty.get(Sample)
    }

    @Suppress("UNCHECKED_CAST") // Callers check cast.
    fun getExtra(sampleName: String, extraKeysJson: String): EventExtras {
        val extraJSON= JSONObject(extraKeysJson)
        val extrasName = "${sampleName}Extras"
        // Search Sample for the Dataclass of "someNameExtras"
        val extraDataClass = Sample.javaClass.kotlin.nestedClasses.first(){
            it.simpleName.equals(extrasName, ignoreCase = true)
        }
        val extra: EventExtras = extraDataClass.createInstance() as EventExtras;
        extraJSON.keys().forEach {
            // QML js uses snake style "panel_cta" while the kotlin codegen
            // uses camel case i.e "panelCta"
            val queryKey = it.replace("_","").lowercase()
            val extraSetter : KMutableProperty<String>? = extra.javaClass.kotlin.members.firstOrNull(){ prop ->
                prop.name.equals(queryKey, ignoreCase = true)
            } as KMutableProperty<String>?
            extraSetter?.setter?.call(extraJSON.getString(it))
        }
        return extra
    }
}
