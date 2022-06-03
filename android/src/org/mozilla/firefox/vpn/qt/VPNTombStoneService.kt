package org.mozilla.firefox.vpn.qt

import android.content.Context
import android.util.Log
import mozilla.components.concept.base.crash.Breadcrumb
import mozilla.components.lib.crash.Crash
import mozilla.components.lib.crash.service.CrashReporterService
import java.io.File
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

class VPNTombStoneService(con: Context) : CrashReporterService {
    private val mContext = con
    override val id: String
        get() = "com.mozilla.firefox.vpn.Tombstone"
    override val name: String
        get() = "VPNTombstoneService"

    override fun createCrashReportUrl(identifier: String): String? {
        return null; // We are not posting this to a webpage
    }

    override fun report(
        throwable: Throwable,
        breadcrumbs: ArrayList<Breadcrumb>
    ): String? {
        writeTombStone(
            """
            Type: GenericThrowable
            Throwable: ${throwable.localizedMessage}
            ${throwable.stackTraceToString()}
            ---- Breadcrumbs
            ${breadcrumbs.map { "${it.category} | ${it.message}" }}
            """.trimIndent()
        )
        return null
    }

    override fun report(crash: Crash.NativeCodeCrash): String? {
        writeTombStone(
            """
            Type: NativeCodeCrash
            Fatal: ${crash.isFatal}
            
            ${crash.breadcrumbs.map {
                "${it.category} | ${it.message}"
            }}
            """.trimIndent()
        )
        return null
    }

    override fun report(crash: Crash.UncaughtExceptionCrash): String? {
        writeTombStone(
            """
            Type: UncaughtExceptionCrash
            Throwable: ${crash.throwable.localizedMessage}
            ${crash.throwable.stackTraceToString()}
            ---- Breadcrumbs
            ${crash.breadcrumbs.map {
                "${it.category} | ${it.message}"
            }}
            """.trimIndent()
        )
        return null
    }

    private fun writeTombStone(content: String) {
        Log.i("BASTI", "writeTombStone")
        val tempDIR = mContext.cacheDir
        val id = getID()
        val file = File(tempDIR, "$id.tombstone.txt")
        file.writeText(
            """
            ****Tombstone-$id****
            $content
            ****Tombstone-$id-****
            """.trimIndent()
        )

        Log.e("MozillaVPN", "Mozilla VPN experienced a crash, $id.tombstone.txt has been written.")
    }

    private fun getID(): String {
        return LocalDateTime.now().format(DateTimeFormatter.ofPattern("y-mm-dd-H-m-ss"))
    }
}
