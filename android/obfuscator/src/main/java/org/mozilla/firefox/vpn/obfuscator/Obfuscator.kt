/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.obfuscator

import com.sun.jna.Callback
import com.sun.jna.Library
import com.sun.jna.Native
import com.sun.jna.Pointer
import com.sun.jna.Structure

/**
 * Kotlin wrapper around the `obfuscators` Rust library.
 *
 * Usage from VPNService:
 *   val obfuscator = Obfuscator.start(
 *       method = Obfuscator.Method.UdpOverTcp,
 *       serverIpv4 = "1.2.3.4",
 *       serverPort = 51820,
 *       ...
 *   )
 *   protect(obfuscator.socketV4)
 *   protect(obfuscator.socketV6)
 *   // rewrite WG endpoint -> 127.0.0.1:relay.localPort
 *   obfuscator.stop()
 */
class Obfuscator private constructor(
    private val handle: Pointer,
    val localPort: Int,
    val socketV4: Int,
    val socketV6: Int,
) {
    @Volatile
    private var stopped = false

    fun stop() {
        synchronized(this) {
            if (stopped) return
            stopped = true
            LIB.obfuscator_stop(handle)
        }
    }

    /** Mirrors `ObfuscationMethod` in obfuscators/src/obfuscator.rs. */
    enum class Method(val value: Int) {
        NoObfuscation(0),
        Lwo(1),
        Masque(2),
        UdpOverTcp(3),
        Shadowsocks(4),
        ;

        companion object {
            fun fromValue(v: Int): Method? = values().firstOrNull { it.value == v }
        }
    }

    companion object {
        /** Mirrors `ObfuscatorConfig` in obfuscators/src/obfuscator.rs. */
        @Structure.FieldOrder(
            "obfuscation_method",
            "server_ipv4_addr_in",
            "server_ipv6_addr_in",
            "server_port",
            "listen_port",
            "server_public_key",
            "public_key",
            "lwo_version",
        )
        open class Config : Structure() {
            @JvmField var obfuscation_method: Int = 0
            @JvmField var server_ipv4_addr_in: String? = null
            @JvmField var server_ipv6_addr_in: String? = null
            @JvmField var server_port: Short = 0
            @JvmField var listen_port: Short = 0
            @JvmField var server_public_key: String? = null
            @JvmField var public_key: String? = null
            @JvmField var lwo_version: Int = 1

            class ByReference : Config(), Structure.ByReference
        }

        interface LogCallback : Callback {
            fun invoke(level: Int, message: Pointer)
        }

        fun interface LogHandler {
            fun onLog(level: Int, message: String)
        }

        private interface Lib : Library {
            fun obfuscator_start(cfg: Config.ByReference): Pointer?
            fun obfuscator_local_port(handle: Pointer): Short
            fun obfuscator_socket_v4(handle: Pointer): Int
            fun obfuscator_socket_v6(handle: Pointer): Int
            fun obfuscator_stop(handle: Pointer)
            fun obfuscators_set_log_handler(handler: LogCallback)
        }

        private val LIB: Lib by lazy {
            Native.load("obfuscators", Lib::class.java)
        }

        // Avoid log callback being collected by GC
        @Volatile
        private var logCallback: LogCallback? = null

        // Install the log handler
        fun setLogHandler(handler: LogHandler) {
            val cb = object : LogCallback {
                override fun invoke(level: Int, message: Pointer) {
                    handler.onLog(level, message.getString(0))
                }
            }
            logCallback = cb
            LIB.obfuscators_set_log_handler(cb)
        }

        // Run the obfuscator, returns null on failure
        fun start(
            method: Method,
            serverIpv4: String?,
            serverIpv6: String?,
            serverPort: Int,
            serverPublicKey: String? = null,
            publicKey: String? = null,
            lwoVersion: Int = 1,
        ): Obfuscator? {
            val cfg = Config.ByReference().apply {
                this.obfuscation_method = method.value
                this.server_ipv4_addr_in = serverIpv4
                this.server_ipv6_addr_in = serverIpv6
                this.server_port = serverPort.toShort()
                this.server_public_key = serverPublicKey
                this.public_key = publicKey
                this.lwo_version = lwoVersion
            }
            val handle = LIB.obfuscator_start(cfg) ?: return null
            val localPort = LIB.obfuscator_local_port(handle).toInt() and 0xFFFF
            val socketV4 = LIB.obfuscator_socket_v4(handle)
            val socketV6 = LIB.obfuscator_socket_v6(handle)
            return Obfuscator(handle, localPort, socketV4, socketV6)
        }
    }
}
