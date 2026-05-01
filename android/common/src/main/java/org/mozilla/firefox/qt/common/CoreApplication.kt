package org.mozilla.firefox.qt.common

import android.app.Activity
import android.os.Bundle
import android.util.Log
import androidx.work.Configuration
import org.mozilla.firefox.qt.common.BuildConfig
import org.qtproject.qt.android.QtApplicationBase

/**
 * CoreApplication - extends QtApplication
 */
class CoreApplication : org.qtproject.qt.android.QtApplicationBase(), Configuration.Provider {
    override val workManagerConfiguration: Configuration
        get() = Configuration.Builder()
            // This is required for Glean to be able to enqueue the PingUploadWorker
            // from both the daemon and the main app.
            .setDefaultProcessName(packageName)
            .setMinimumLoggingLevel(Log.INFO)
            .build()

    override fun onCreate() {
        super.onCreate()
        instance = this
    }

    companion object {
        private val tag = "CoreApplicationQt"
        lateinit var instance: CoreApplication
            private set
    }
}
