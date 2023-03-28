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
}
