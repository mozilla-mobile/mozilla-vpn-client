/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt

import android.annotation.SuppressLint
import android.content.ContentValues
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.provider.MediaStore

object VPNShareUtils {
    @SuppressLint("NewApi")
    @JvmStatic
    fun sharePlainText(text: String) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            // Not supported on oldies. :c
            return
        }
        val ctx: Context = VPNActivity.getInstance()
        val resolver = ctx.contentResolver

        // Find the right volume to use:
        val collection = MediaStore.Downloads.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY)

        val fileMetaData = ContentValues().apply {
            put(MediaStore.Downloads.MIME_TYPE, "text/plain")
            put(MediaStore.Downloads.DISPLAY_NAME, "MozillaVPN Logs")
            put(MediaStore.Downloads.IS_PENDING, 1)
        }
        // Create the File and get the URI
        val fileURI: Uri? = resolver.insert(collection, fileMetaData)
        fileURI?.let { fileURI ->
            val tx = resolver.openOutputStream(fileURI)
            tx?.writer(Charsets.UTF_8)?.write(text)
            tx?.close()
            // Now update the Files meta data that the file exists
            fileMetaData.clear()
            fileMetaData.put(MediaStore.Downloads.IS_PENDING, 0)
            resolver.update(fileURI, fileMetaData, null, null)

            val sendIntent = Intent(Intent.ACTION_SEND)
            sendIntent.putExtra(Intent.EXTRA_STREAM, fileURI)
            sendIntent.setType("text/plain")

            val chooseIntent = Intent.createChooser(sendIntent, "Share Logs")
            ctx.startActivity(chooseIntent)
        }
    }
}
