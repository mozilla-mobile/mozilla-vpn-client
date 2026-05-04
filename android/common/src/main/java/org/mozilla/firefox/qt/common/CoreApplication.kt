package org.mozilla.firefox.qt.common

import android.app.Activity
import android.os.Bundle
import android.util.Log
import org.mozilla.firefox.qt.common.BuildConfig
import org.qtproject.qt.android.QtApplicationBase

/**
 * CoreApplication - extends QtApplication
 */
class CoreApplication : org.qtproject.qt.android.QtApplicationBase() {
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
