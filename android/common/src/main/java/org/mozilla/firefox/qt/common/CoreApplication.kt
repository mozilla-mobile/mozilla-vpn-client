package org.mozilla.firefox.qt.common

import android.app.Activity
import android.os.Bundle
import android.util.Log
import androidx.work.Configuration
import com.adjust.sdk.*

/**
 * CoreApplication - extends QtApplication with callbacks to hook in adjust and
 * a privacy proxy that minimizes tracking.
 */
class CoreApplication : org.qtproject.qt.android.bindings.QtApplication(), Configuration.Provider {
    override fun getWorkManagerConfiguration(): Configuration {
        return Configuration.Builder()
            // This is required for Glean to be able to enqueue the PingUploadWorker
            // from both the daemon and the main app.
            .setDefaultProcessName(packageName)
            .setMinimumLoggingLevel(Log.INFO)
            .build()
    }

    override fun onCreate() {
        super.onCreate()
        instance = this
    }

    /**
     * Is true if adjust is configured and also
     * has been properly initialized
     */
    private var adjustActive = false

    companion object {
        private val tag = "CoreApplicationAdjust"
        lateinit var instance: CoreApplication
            private set

        /**
         * initializeAdjust
         * Init's the adjust framework.
         * @param inProduction - If false, Adjust Sandbox Mode is enabled
         * @param proxyPort - Port of the Proxy that cleans up Adjust tracking data.
         */
        @JvmStatic
        fun initializeAdjust(inProduction: Boolean, proxyPort: Int) {
            Log.i(tag, "Initializing Adjust")
            val appToken: String = BuildConfig.ADJUST_SDK_TOKEN
            val environment: String =
                if (inProduction) AdjustConfig.ENVIRONMENT_PRODUCTION else AdjustConfig.ENVIRONMENT_SANDBOX
            val config = AdjustConfig(instance, appToken, environment)
            config.setLogLevel(LogLevel.DEBUG)
            config.setSendInBackground(true)
            AdjustFactory.setBaseUrl("http://127.0.0.1:$proxyPort")
            AdjustFactory.setGdprUrl("http://127.0.0.1:$proxyPort")
            Adjust.onCreate(config)

            instance.registerActivityLifecycleCallbacks(AdjustLifecycleCallbacks())
            instance.adjustActive = true
        }

        @JvmStatic
        fun trackEvent(event: String) {
            if (event.isNullOrEmpty()) {
                return
            }
            if (!instance.adjustActive) {
                return
            }
            val adjustEvent = AdjustEvent(event)
            Adjust.trackEvent(adjustEvent)
        }

        /**
         * Issues a GDPR delete request.
         * @param activity - Need's to provide the requesting activity.
         */
        @JvmStatic
        fun forget(activity: Activity?) {
            if (!instance.adjustActive) {
                return
            }
            Adjust.gdprForgetMe(activity)
        }
    }
    private class AdjustLifecycleCallbacks : ActivityLifecycleCallbacks {
        override fun onActivityResumed(activity: Activity) {
            Adjust.onResume()
        }

        override fun onActivityPaused(activity: Activity) {
            Adjust.onPause()
        }

        override fun onActivityStopped(p0: Activity) {}
        override fun onActivitySaveInstanceState(p0: Activity, p1: Bundle) {}
        override fun onActivityDestroyed(p0: Activity) {}
        override fun onActivityCreated(p0: Activity, p1: Bundle?) {}
        override fun onActivityStarted(p0: Activity) {}
    }
}
