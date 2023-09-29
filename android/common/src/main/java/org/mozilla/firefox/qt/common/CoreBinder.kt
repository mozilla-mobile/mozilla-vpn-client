package org.mozilla.firefox.qt.common

import android.os.Binder

open class CoreBinder : Binder() {
    /** The codes this Binder does accept in [onTransact] */
    object ACTIONS {
        const val activate = 1
        const val deactivate = 2
        const val registerEventListener = 3
        const val requestStatistic = 4
        const val requestCleanupLog = 6
        const val resumeActivate = 7
        const val setNotificationText = 8
        const val recordEvent = 10
        const val getStatus = 13
        const val setStartOnBoot = 15
        const val reactivate = 16
        const val clearStorage = 17
        const val setGleanUploadEnabled = 18
        const val notificationPermissionFired = 19
    }

    /** The codes we Are Using in case of [dispatchEvent] */
    object EVENTS {
        const val init = 0
        const val connected = 1
        const val disconnected = 2
        const val statisticUpdate = 3
        const val activationError = 5
        const val permissionRequired = 6
        const val requestGleanUploadEnabledState = 7
        const val requestNotificationPermission = 8
        const val onboardingCompleted = 9
    }
}
