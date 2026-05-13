package org.mozilla.firefox.qt.common

import android.os.Binder

open class CoreBinder : Binder() {
    /** The codes this Binder does accept in [onTransact] */
    object ACTIONS {
        const val activate = 1
        const val deactivate = 2
        const val registerEventListener = 3
        const val requestStatistic = 4
        const val requestCleanupLog = 5
        const val resumeActivate = 6
        const val setNotificationText = 7
        const val getStatus = 8
        const val setStartOnBoot = 9
        const val reactivate = 10
        const val clearStorage = 11
        const val notificationPermissionFired = 12
        const val silentServerSwitch = 13
    }

    /** The codes we Are Using in case of [dispatchEvent] */
    object EVENTS {
        const val init = 0
        const val connected = 1
        const val disconnected = 2
        const val statisticUpdate = 3
        const val activationError = 4
        const val permissionRequired = 5
        const val requestNotificationPermission = 6
        const val onboardingCompleted = 7
    }
}
