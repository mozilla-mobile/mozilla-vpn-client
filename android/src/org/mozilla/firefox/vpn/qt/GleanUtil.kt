package org.mozilla.firefox.vpn.qt

import android.util.Log
import androidx.annotation.MainThread
import mozilla.components.service.glean.BuildInfo
import mozilla.components.service.glean.Glean
import mozilla.components.service.glean.config.Configuration
import mozilla.telemetry.glean.net.HttpURLConnectionUploader
import mozilla.telemetry.glean.private.EventMetricType
import mozilla.telemetry.glean.private.NoExtraKeys
import mozilla.telemetry.glean.private.NoExtras
import org.json.JSONObject
import org.mozilla.firefox.vpn.GleanMetrics.Pings
import org.mozilla.firefox.vpn.GleanMetrics.Sample
import java.util.*
import kotlin.reflect.KProperty1


//@MainThread
@Suppress("UNUSED") // All the calls are via jni from the client, so we can ignore
object GleanUtil {
    @JvmStatic
    fun initializeGlean(upload:Boolean =false, channel:String = "staging"){
        val conf =Configuration(HttpURLConnectionUploader(),Configuration.DEFAULT_TELEMETRY_ENDPOINT,channel,null);
        val build = BuildInfo("versionCode","VersionName", Calendar.getInstance())
        Glean.registerPings(Pings);
        Glean.initialize(
            applicationContext = VPNActivity.getInstance().applicationContext,
            uploadEnabled = upload,
            buildInfo = build,
            configuration = conf
        );
        Log.i("Android-Glean", "Initialised android glean")
    }
    @JvmStatic
    fun setGleanUploadEnabled(upload: Boolean){
        Glean.setUploadEnabled(upload);
    }
    @JvmStatic
    fun setGleanSourceTag(tags:String = "") {
        //val tag_list = tag.split(",")
        //Glean.setSourceTags(tag_list);
        // TODO: Glean-Android does not support this, yet.
        // We're using this only for functional tests, so no problem i guess c:
        return;
    }
    @JvmStatic
    fun sendGleanPings(){
        Pings.main.submit()
    }
    @JvmStatic
    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEvent(sampleName:String) {
        val sample = getSample(sampleName) as? EventMetricType<NoExtraKeys,NoExtras>;
        if(sample == null){
            Log.e("ANDROID-GLEAN", "Ping not found $sampleName");
            return;
        }
        sample.record(NoExtras());
    }
    @JvmStatic
    @Suppress("UNCHECKED_CAST") // We're using nullable casting and check that :)
    fun recordGleanEventWithExtraKeys(sampleName:String, extraKeysJson:String) {
        val extraKeys = JSONObject(extraKeysJson)
        val sample = getSample(sampleName) as? EventMetricType<NoExtraKeys,NoExtras>;
        if(sample == null){
            Log.e("ANDROID-GLEAN", "Ping not found $sampleName");
            return;
        }
        sample.record(buildMap {
            extraKeys.keys().forEach {
                put(NoExtraKeys.valueOf(it),extraKeys.get(it).toString())
            }
        })
        return;
    }
    @Suppress("UNCHECKED_CAST") // Callers check cast.
    fun getSample(sampleName: String):EventMetricType<*,*>{
        val sampleProperty =  Sample.javaClass.kotlin.members.first {
            it.name == sampleName
        } as KProperty1<Sample, EventMetricType<*,*>>
        val sampleInstance = sampleProperty.get(Sample);
        return sampleInstance;
    };
}