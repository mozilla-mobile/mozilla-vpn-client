package org.mozilla.firefox.vpn.qt

import android.content.Context
import android.content.Intent
import android.util.Log
import mozilla.components.lib.crash.CrashReporter
import mozilla.components.lib.crash.handler.CrashHandlerService

class VPNCrashReporterUtil(context: Context) {
    init {
        val reporter = CrashReporter(
            context,
            // Always prompt
            shouldPrompt = CrashReporter.Prompt.ALWAYS,
            services = listOf(
                VPNTombStoneService(context)
            )
        )
        reporter.install(context)
        Log.i("BASTI", "INSTALLED REPORTER")
    }
    companion object {

        const val ACTION_CRASHED = "org.mozilla.gecko.ACTION_CRASHED"
        const val EXTRA_MINIDUMP_PATH = "minidumpPath"
        const val EXTRA_EXTRAS_PATH = "extrasPath"
        const val EXTRA_CRASH_PROCESS_TYPE = "processType"

        @JvmStatic
        @Suppress("unused", "DIVISION_BY_ZERO") // JNI caller
        fun testUncaughtException() {
            val oops = 10 / 0
        }
        @JvmStatic
        @Suppress("unused") // JNI caller
        fun testNullAccessException() {
            val ctx: Context? = null
            ctx!!.openFileInput("HELLO NULL")
        }

        @JvmStatic
        @Suppress("unused") // JNI caller
        fun nativePanic(miniDumpFilepath: String) {
            // Note to self, this is undocumented ... i found this in gecko.
            val context = VPNActivity.getInstance()
            Log.e("BASTI", miniDumpFilepath)

            val i = Intent(ACTION_CRASHED, null, context, CrashHandlerService::class.java).apply {
                putExtra(EXTRA_MINIDUMP_PATH, miniDumpFilepath)
                putExtra(EXTRA_EXTRAS_PATH, "") // We don't have extras for now
                putExtra(EXTRA_CRASH_PROCESS_TYPE, "MAIN") // We only have the QT Process attached to this.
            }
            context.startService(i)
        }
    }
}
