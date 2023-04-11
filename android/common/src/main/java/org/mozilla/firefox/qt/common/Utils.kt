/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.qt.common
import android.annotation.SuppressLint
import android.content.Context
import android.app.Activity
import android.content.Intent
import android.util.Log
import mozilla.telemetry.glean.BuildInfo
import mozilla.telemetry.glean.Glean
import mozilla.telemetry.glean.config.Configuration
import org.bouncycastle.asn1.ASN1Sequence
import org.bouncycastle.asn1.pkcs.RSAPublicKey
import java.security.KeyFactory
import java.security.Signature
import java.security.spec.RSAPublicKeySpec
import java.util.*
import android.annotation.SuppressLint
import android.content.ContentValues
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.provider.MediaStore
import android.provider.Settings
import android.util.Log
import java.io.IOException
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter
import java.util.Calendar

// Companion for Utils.cpp
object Utils {

    @SuppressLint("Unused")
    @JvmStatic
    fun verifyContentSignature(publicKey: ByteArray, content: ByteArray, signature: ByteArray): Boolean {
        return try {
            val sig = Signature.getInstance("SHA256withRSA")
            // Use bountycastle to parse the openssl-rsa file
            val pk: RSAPublicKey =
                RSAPublicKey.getInstance(ASN1Sequence.fromByteArray(publicKey))
            // Pass this to android signing stuff :)
            val spec = RSAPublicKeySpec(pk.modulus, pk.publicExponent)
            val kf: KeyFactory = KeyFactory.getInstance("RSA")
            sig.initVerify(kf.generatePublic(spec))

            sig.update(content)
            sig.verify(signature)
        } catch (e: Exception) {
            Log.e("VPNUtils", "Signature Exception $e")
            false
        }
    }
    @SuppressLint("NewApi")
    @JvmStatic
    fun initializeGlean(ctx: Context, isTelemetryEnabled: Boolean, channel: String) {
        Glean.initialize(
            applicationContext = ctx.applicationContext,
            uploadEnabled = isTelemetryEnabled,
            // GleanBuildInfo can only be generated for application,
            // We are in a library so we have to build it ourselves.
            buildInfo = BuildInfo(
                BuildConfig.VERSIONCODE,
                BuildConfig.SHORTVERSION,
                Calendar.getInstance(),
            ),
            configuration = Configuration(channel = channel),
        )
    }

    @JvmStatic
    fun launchPlayStore(activity: Activity) {
        val intent = Intent.makeMainSelectorActivity(
            Intent.ACTION_MAIN,
            Intent.CATEGORY_APP_MARKET
        )
        activity.startActivity(intent)
    }

    @SuppressLint("NewApi")
    @JvmStatic
    fun sharePlainText(text: String): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            // Not supported on oldies. :c
            return false
        }
        val ctx: Context = VPNActivity.getInstance()
        val resolver = ctx.contentResolver

        // Find the right volume to use:
        val collection = MediaStore.Downloads.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY)
        val dateTime = LocalDateTime.now().format(DateTimeFormatter.ofPattern("y-mm-dd-H-m-ss"))
        val fileMetaData =
            ContentValues().apply {
                put(MediaStore.Downloads.MIME_TYPE, "text/plain")
                put(MediaStore.Downloads.DISPLAY_NAME, "MozillaVPN_Logs_$dateTime")
                put(MediaStore.Downloads.IS_PENDING, 1)
            }
        // Create the File and get the URI
        val fileURI: Uri? = resolver.insert(collection, fileMetaData)
        if (fileURI == null) {
            return false
        }

        val tx = resolver.openOutputStream(fileURI)
        if (tx == null) {
            return false
        }
        try {
            val writer = tx.writer(Charsets.UTF_8)
            writer?.write(text)
            writer?.flush()
        } catch (e: IOException) {
            return false
        }
        tx.flush()
        tx.close()
        // Now update the Files meta data that the file exists
        fileMetaData.clear()
        fileMetaData.put(MediaStore.Downloads.IS_PENDING, 0)

        try {
            val ok = resolver.update(fileURI, fileMetaData, null, null)
            if (ok == 0) {
                Log.e("MozillaVPNLogs", "resolver update - err: 0 Rows updated")
            }
        } catch (e: Exception) {
            Log.e("MozillaVPNLogs", "resolver update - exception: " + e.message)
        }

        val sendIntent = Intent(Intent.ACTION_SEND)
        sendIntent.putExtra(Intent.EXTRA_STREAM, fileURI)
        sendIntent.setType("*/*")

        val chooseIntent = Intent.createChooser(sendIntent, "Share Logs")
        ctx.startActivity(chooseIntent)
        return true
    }
}
