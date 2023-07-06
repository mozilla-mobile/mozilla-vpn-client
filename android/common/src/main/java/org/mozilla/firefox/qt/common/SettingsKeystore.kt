package org.mozilla.firefox.qt.common

import android.annotation.SuppressLint

@SuppressLint("Unused") // This is used via jni
object SettingsKeystore {
    private const val PREF_KEY = "vpn.qt.settings.encryptionKey"

    @JvmStatic
    fun hasKey(): Boolean {
        return Prefs.get(CoreApplication.instance.applicationContext).contains(PREF_KEY)
    }

    @JvmStatic
    fun setKey(value: String) {
        Prefs.get(CoreApplication.instance.applicationContext).edit().apply {
            putString(PREF_KEY, value)
        }.apply()
    }

    @JvmStatic
    fun getKey(): String {
        return Prefs.get(CoreApplication.instance.applicationContext).getString(PREF_KEY, "")!!
    }

    @JvmStatic
    fun clear() {
        Prefs.get(CoreApplication.instance.applicationContext).edit().apply {
            remove(PREF_KEY)
        }.apply()
    }
}
