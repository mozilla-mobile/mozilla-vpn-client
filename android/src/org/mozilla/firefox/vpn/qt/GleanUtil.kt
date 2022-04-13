package org.mozilla.firefox.vpn.qt

import androidx.annotation.MainThread
import mozilla.components.service.glean.BuildInfo
import mozilla.components.service.glean.Glean
import mozilla.components.service.glean.config.Configuration
import mozilla.telemetry.glean.net.HttpURLConnectionUploader
import mozilla.telemetry.glean.private.EventMetricType
import mozilla.telemetry.glean.private.NoExtraKeys
import mozilla.telemetry.glean.private.NoExtras
import org.mozilla.firefox.vpn.GleanMetrics.Pings
import org.mozilla.firefox.vpn.GleanMetrics.Sample
import java.util.*
import kotlin.reflect.KProperty1


//@MainThread
@Suppress("UNUSED") // All the calls are via jni
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
    }
    @JvmStatic
    fun setGleanUploadEnabled(upload: Boolean){
        Glean.setUploadEnabled(upload);
    }
    @JvmStatic
    fun setGleanSourceTag(tag:String = "") {
        val tag_list = tag.split(",")
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
    fun recordGleanEvent(sampleName:String) {

        val sample = getSample(sampleName)
        (sample as? EventMetricType<NoExtraKeys,NoExtras>)?.record(NoExtras())
    }
    @JvmStatic
    fun onRecordGleanEventWithExtraKeys(sample:String, extrakeys:String) {
        // TODO: Glean-Android does not support this, yet.
        // We're using this only for functional tests, so no problem i guess c:
        return;
    }
    @Suppress("UNCHECKED_CAST")
    fun getSample(sampleName: String):EventMetricType<*,*>{
        val sampleProperty =  Sample.javaClass.kotlin.members.first { it.name == sampleName } as KProperty1<Sample, EventMetricType<*,*>>
        val sampleInstance = sampleProperty.get(Sample);
        return sampleInstance;
    };
}