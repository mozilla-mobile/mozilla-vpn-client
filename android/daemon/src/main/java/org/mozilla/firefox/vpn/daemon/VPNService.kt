/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.CountDownTimer
import android.os.IBinder
import android.system.OsConstants
import com.wireguard.android.util.SharedLibraryLoader
import com.wireguard.config.Config
import com.wireguard.config.InetEndpoint
import com.wireguard.config.InetNetwork
import com.wireguard.config.Interface
import com.wireguard.config.Peer
import com.wireguard.crypto.Key
import org.json.JSONObject
import java.util.*

class VPNService : android.net.VpnService() {
    private val tag = "VPNService"
    private var mBinder: VPNServiceBinder = VPNServiceBinder(this)
    val mGlean = GleanUtil(this)
    private var mConfig: JSONObject? = null
    private var mConnectionTime: Long = 0
    private var mAlreadyInitialised = false
    private val controllerPeriodicStateRecorderMsec: Long = 10800000

    private val mGleanTimer = object : CountDownTimer(
        controllerPeriodicStateRecorderMsec,
        controllerPeriodicStateRecorderMsec / 4
    ) {
        override fun onTick(millisUntilFinished: Long) {}
        override fun onFinish() {
            Log.i(tag, "Timer Done!")
            if (currentTunnelHandle == -1) {
                mGlean.recordGleanEvent("controllerStateOff")
            } else {
                // When we're stil connected, rescheudle.
                this.start()
                mGlean.recordGleanEvent("controllerStateOn")
            }
        }
    }

    private var currentTunnelHandle = -1

    fun init() {
        if (mAlreadyInitialised) {
            return
        }
        Log.init(this)
        mGlean.initializeGlean()
        SharedLibraryLoader.loadSharedLibrary(this, "wg-go")
        Log.i(tag, "loaded lib")
        Log.e(tag, "Wireguard Version ${wgVersion()}")
        mAlreadyInitialised = true
    }

    override fun onUnbind(intent: Intent?): Boolean {
        if (!isUp) {
            // If the Qt Client got closed while we were not connected
            // we do not need to stay as a foreground service.
            stopForeground(true)
        }
        return super.onUnbind(intent)
    }

    /**
     * EntryPoint for the Service, gets Called when AndroidController.cpp
     * calles bindService. Returns the [VPNServiceBinder] so QT can send Requests to it.
     */
    override fun onBind(intent: Intent?): IBinder? {
        Log.v(tag, "Got Bind request")
        init()
        return mBinder
    }

    /**
     * Might be the entryPoint if the Service gets Started via an
     * Service Intent: Might be from Always-On-Vpn from Settings
     * or from Booting the device and having "connect on boot" enabled.
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        init()
        intent?.let {
            if (intent.getBooleanExtra("startOnly", false)) {
                Log.i(tag, "Start only!")
                // If this is a Start Only request, the client will soon
                // bind to the service anyway.
                // We should return START_NOT_STICKY so that after an unbind()
                // the OS will not try to restart the service.
                return START_NOT_STICKY
            }
        }
        // This start is from always-on
        if (this.mConfig == null) {
            // We don't have tunnel to turn on - Try to create one with last config the service got
            val prefs = Prefs.get(this)
            val lastConfString = prefs.getString("lastConf", "")
            if (lastConfString.isNullOrEmpty()) {
                // We have nothing to connect to -> Exit
                Log.e(
                    tag,
                    "VPN service was triggered without defining a Server or having a tunnel"
                )
                return super.onStartCommand(intent, flags, startId)
            }
            this.mConfig = JSONObject(lastConfString)
        }
        try {
            turnOn(this.mConfig!!)
        } catch (error: Exception) {
            Log.e(tag, "Failed to start the VPN for always-on:")
            Log.e(tag, error.toString())
            Log.stack(tag, error.stackTrace)
        }

        return super.onStartCommand(intent, flags, startId)
    }

    // Invoked when the application is revoked.
    // At this moment, the VPN interface is already deactivated by the system.
    override fun onRevoke() {
        this.turnOff()
        super.onRevoke()
    }

    var connectionTime: Long = 0
        get() {
            return mConnectionTime
        }

    var isUp: Boolean
        get() {
            return currentTunnelHandle >= 0
        }
        private set(value) {
            if (value) {
                mBinder.dispatchEvent(VPNServiceBinder.EVENTS.connected, "")
                mConnectionTime = System.currentTimeMillis()
                return
            }
            mBinder.dispatchEvent(VPNServiceBinder.EVENTS.disconnected, "")
            mConnectionTime = 0
        }
    val status: JSONObject
        get() {
            val deviceIpv4: String = ""
            return JSONObject().apply {
                putOpt("rx_bytes", getConfigValue("rx_bytes")?.toInt())
                putOpt("tx_bytes", getConfigValue("tx_bytes")?.toInt())
                putOpt("endpoint", mConfig?.getJSONObject("server")?.getString("ipv4Gateway"))
                putOpt("deviceIpv4", mConfig?.getJSONObject("device")?.getString("ipv4Address"))
            }
        }
      /*
      * Checks if the VPN Permission is given.
      * If the permission is given, returns true
      * Requests permission and returns false if not.
      */
    fun checkPermissions(): Boolean {
        // See https://developer.android.com/guide/topics/connectivity/vpn#connect_a_service
        // Call Prepare, if we get an Intent back, we dont have the VPN Permission
        // from the user. So we need to pass this to our main Activity and exit here.
        val intent = prepare(this)
        if (intent == null) {
            Log.e(tag, "VPN Permission Already Present")
            return true
        }
        Log.e(tag, "Requesting VPN Permission")
        return false
    }

    fun turnOn(json: JSONObject) {
        Log.sensitive(tag, json.toString())
        val wireguard_conf = buildWireugardConfig(json)

        if (!checkPermissions()) {
            Log.e(tag, "turn on was called without no permissions present!")
            isUp = false
            return
        }
        Log.i(tag, "Permission okay")
        if (currentTunnelHandle != -1) {
            Log.e(tag, "Tunnel already up")
            // Turn the tunnel down because this might be a switch
            wgTurnOff(currentTunnelHandle)
        }
        val wgConfig: String = wireguard_conf!!.toWgUserspaceString()
        val builder = Builder()
        setupBuilder(wireguard_conf, builder)
        builder.setSession("mvpn0")
        builder.establish().use { tun ->
            if (tun == null)return
            Log.i(tag, "Go backend " + wgVersion())
            currentTunnelHandle = wgTurnOn("mvpn0", tun.detachFd(), wgConfig)
        }
        if (currentTunnelHandle < 0) {
            Log.e(tag, "Activation Error Code -> $currentTunnelHandle")
            isUp = false
            return
        }
        protect(wgGetSocketV4(currentTunnelHandle))
        protect(wgGetSocketV6(currentTunnelHandle))
        mConfig = json
        isUp = true

        // Store the config in case the service gets
        // asked boot vpn from the OS
        val prefs = Prefs.get(this)
        prefs.edit()
            .putString("lastConf", json.toString())
            .apply()

        NotificationUtil.get(this)?.show(this) // Go foreground
        mGleanTimer.start()
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        wgTurnOff(currentTunnelHandle)
        currentTunnelHandle = -1
        stopForeground(false)
        isUp = false
        mGleanTimer.cancel()
    }

    /**
     * Configures an Android VPN Service Tunnel
     * with a given Wireguard Config
     */
    private fun setupBuilder(config: Config, builder: Builder) {
        // Setup Split tunnel
        for (excludedApplication in config.`interface`.excludedApplications)
            builder.addDisallowedApplication(excludedApplication)

        // Device IP
        for (addr in config.`interface`.addresses) builder.addAddress(addr.address, addr.mask)
        // DNS
        for (addr in config.`interface`.dnsServers) builder.addDnsServer(addr.hostAddress)
        // Add All routes the VPN may route tos
        for (peer in config.peers) {
            for (addr in peer.allowedIps) {
                builder.addRoute(addr.address, addr.mask)
            }
        }
        builder.allowFamily(OsConstants.AF_INET)
        builder.allowFamily(OsConstants.AF_INET6)
        builder.setMtu(config.`interface`.mtu.orElse(1280))

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) builder.setMetered(false)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) setUnderlyingNetworks(null)

        builder.setBlocking(true)
    }

    /**
     * Gets config value for {key} from the Current
     * running Wireguard tunnel
     */
    private fun getConfigValue(key: String): String? {
        if (!isUp) {
            return null
        }
        val config = wgGetConfig(currentTunnelHandle) ?: return null
        val lines = config.split("\n")
        for (line in lines) {
            val parts = line.split("=")
            val k = parts.first()
            val value = parts.last()
            if (key == k) {
                return value
            }
        }
        return null
    }

    /**
     * Create a Wireguard [Config]  from a [json] string -
     * The [json] will be created in AndroidController.cpp
     */
    private fun buildWireugardConfig(obj: JSONObject): Config {
        val confBuilder = Config.Builder()
        val jServer = obj.getJSONObject("server")
        val peerBuilder = Peer.Builder()
        val ep =
            InetEndpoint.parse(jServer.getString("ipv4AddrIn") + ":" + jServer.getString("port"))
        peerBuilder.setEndpoint(ep)
        peerBuilder.setPublicKey(Key.fromBase64(jServer.getString("publicKey")))

        val jAllowedIPList = obj.getJSONArray("allowedIPs")
        if (jAllowedIPList.length() == 0) {
            val internet = InetNetwork.parse("0.0.0.0/0") // aka The whole internet.
            peerBuilder.addAllowedIp(internet)
        } else {
            (0 until jAllowedIPList.length()).toList().forEach {
                val network = InetNetwork.parse(jAllowedIPList.getString(it))
                peerBuilder.addAllowedIp(network)
            }
        }

        confBuilder.addPeer(peerBuilder.build())

        val privateKey = obj.getJSONObject("keys").getString("privateKey")
        val jDevice = obj.getJSONObject("device")

        val ifaceBuilder = Interface.Builder()
        ifaceBuilder.parsePrivateKey(privateKey)
        ifaceBuilder.addAddress(InetNetwork.parse(jDevice.getString("ipv4Address")))
        ifaceBuilder.addAddress(InetNetwork.parse(jDevice.getString("ipv6Address")))
        ifaceBuilder.addDnsServer(InetNetwork.parse(obj.getString("dns")).address)
        val jExcludedApplication = obj.getJSONArray("excludedApps")
        (0 until jExcludedApplication.length()).toList().forEach {
            val appName = jExcludedApplication.get(it).toString()
            ifaceBuilder.excludeApplication(appName)
        }
        confBuilder.setInterface(ifaceBuilder.build())
        return confBuilder.build()
    }

    companion object {
        @JvmStatic
        fun startService(c: Context) {
            c.applicationContext.startService(
                Intent(c.applicationContext, VPNService::class.java).apply {
                    putExtra("startOnly", true)
                }
            )
        }

        @JvmStatic
        private external fun wgGetConfig(handle: Int): String?
        @JvmStatic
        private external fun wgGetSocketV4(handle: Int): Int
        @JvmStatic
        private external fun wgGetSocketV6(handle: Int): Int
        @JvmStatic
        private external fun wgTurnOff(handle: Int)
        @JvmStatic
        private external fun wgTurnOn(ifName: String, tunFd: Int, settings: String): Int
        @JvmStatic
        private external fun wgVersion(): String?
    }
}
