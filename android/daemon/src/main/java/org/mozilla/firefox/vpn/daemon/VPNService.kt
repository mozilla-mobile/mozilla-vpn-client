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
import com.wireguard.config.Config
import com.wireguard.config.InetEndpoint
import com.wireguard.config.InetNetwork
import com.wireguard.config.Interface
import com.wireguard.config.Peer
import com.wireguard.crypto.Key
import mozilla.telemetry.glean.BuildInfo
import mozilla.telemetry.glean.Glean
import mozilla.telemetry.glean.config.Configuration
import org.json.JSONObject
import org.mozilla.firefox.qt.common.CoreBinder
import org.mozilla.firefox.qt.common.Prefs
import org.mozilla.firefox.vpn.daemon.GleanMetrics.ConnectionHealth
import org.mozilla.firefox.vpn.daemon.GleanMetrics.Pings
import org.mozilla.firefox.vpn.daemon.GleanMetrics.Session
import org.mozilla.guardian.tunnel.WireGuardGo
import java.io.File
import java.util.*

class VPNService : android.net.VpnService() {
    private val tag = "VPNService"
    private var mBinder: VPNServiceBinder = VPNServiceBinder(this)
    val mNotificationHandler by lazy { NotificationUtil(this) }
    private var mConfig: JSONObject? = null
    private var mConnectionTime: Long = 0
    private var mAlreadyInitialised = false
    val mConnectionHealth = ConnectionHealth(this)
    private var mCityname = ""
    private val mBackgroundPingTimerMSec: Long = 3 * 60 * 60 * 1000 // 3 hours, in milliseconds
    private val mShortTimerBackgroundPingMSec: Long = 3 * 60 * 1000 // 3 minutes, in milliseconds

    private var currentServerConfig = 0

    // `isUsingShortTimerSessionPing` is not reliable until mConfig has been set, so it cannot be set upon
    // class initialization (VPN-6629). And this cannot be a lazy var, as that caused a crash (PR #8555)
    // Thus, the interval ticks are for the debug timer mode (but `isUsingShortTimerSessionPing` determines
    // if a ping is actually sent), and countdown finishes are for typical sending times.
    private val mMetricsTimer: CountDownTimer = object : CountDownTimer(mBackgroundPingTimerMSec, mShortTimerBackgroundPingMSec) {
        override fun onTick(millisUntilFinished: Long) {
            // We want to skip the first onTick, which is called as the timer is started.
            // (The Start ping is sent then, so there is no need for a Timer ping.)
            var wasTimerJustStarted = (millisUntilFinished >= (mBackgroundPingTimerMSec - 1000))
            if (!wasTimerJustStarted && isUsingShortTimerSessionPing && isSuperDooperMetricsActive) {
                Log.i(tag, "Sending daemon_timer ping (on short timer debug schedule)")
                Pings.daemonsession.submit(
                    Pings.daemonsessionReasonCodes.daemonTimer,
                )
            }
        }
        override fun onFinish() {
            Log.i(tag, "Sending daemon_timer ping")
            if (isSuperDooperMetricsActive) {
                Pings.daemonsession.submit(
                    Pings.daemonsessionReasonCodes.daemonTimer,
                )
            }
            this.start()
        }
    }

    private val isSuperDooperMetricsActive: Boolean
        get() {
            return this.mConfig?.optBoolean("isSuperDooperFeatureActive", false) ?: false
        }

    private val isServerLocatedInUserCountry: Boolean?
        get() {
            if (this.mConfig?.isNull("serverLocatedInUserCountry") ?: false) {
                return null
            } else {
                return this.mConfig?.optBoolean("serverLocatedInUserCountry")
            }
        }

    // Is a VPN connection coming from the app (not daemon) AND
    // is happening because of a server switch?
    private val isAppChangingServers: Boolean
        get() {
            // could be user choosing new server or silent switching
            return (this.mConfig?.optInt("reason") ?: 0) == 1
        }

    private val isUsingShortTimerSessionPing: Boolean
        get() {
            return this.mConfig?.optBoolean("isUsingShortTimerSessionPing", false) ?: false
        }

    private val gleanDebugTag: String?
        get() {
            val gleanDebugTag = this.mConfig?.optString("gleanDebugTag", "") ?: ""
            if (!(gleanDebugTag.isEmpty())) {
                return gleanDebugTag
            } else {
                return null
            }
        }

    private var currentTunnelHandle = -1
        set(value: Int) {
            field = value
            if (value > -1) {
                if (!isAppChangingServers) {
                    // Don't reset the timer on a server switch
                    mConnectionTime = System.currentTimeMillis()
                }
                Log.i(tag, "Dispatch Daemon State -> connected")
                mBinder.dispatchEvent(
                    CoreBinder.EVENTS.connected,
                    JSONObject()
                        .apply {
                            put("city", mCityname)
                        }
                        .toString(),
                )
                return
            }
            Log.i(tag, "Dispatch Daemon State -> disconnected")
            mBinder.dispatchEvent(CoreBinder.EVENTS.disconnected, "")
            mConnectionTime = 0
        }

    fun init() {
        if (mAlreadyInitialised) {
            Log.i(tag, "VPN Service already initialized, ignoring.")
            return
        }
        Log.init(this)
        // No need to load the library here; WireGuardGo does it statically.
        Log.i(tag, "Initialised Service with Wireguard Version ${WireGuardGo.wgVersion()}")

        // Check in with wg, if there is a tunnel.
        // This should be 99% -1, however if the service get's destroyed and the
        // wireguard tunnel lives on, we can recover from here :)
        currentTunnelHandle = WireGuardGo.wgGetLatestHandle()
        Log.i(tag, "Wireguard reported current tunnel: $currentTunnelHandle")
        mAlreadyInitialised = true

        // It's usually a bad practice to initialize Glean with the wrong
        // value for uploadEnabled... However, since this is a very controlled
        // situation -- it should only happen when logging in to a brand new
        // installation of the app -- we should be fine
        //
        // If the `glean_enabled` preference is not set, when a new event listener
        // is bound to this service, it will request that the main app broadcast
        // the user provided preference for this. So it should not be long until
        // the correct value is set here. See VPNServiceBinder > onTransact > ACTIONS.registerEventListener.
        initializeGlean(Prefs.get(this).getBoolean("glean_enabled", false))
    }

    override fun onUnbind(intent: Intent?): Boolean {
        if (!isUp) {
            Log.v(tag, "Client Disconnected, VPN is down - Service might shut down soon")
            return super.onUnbind(intent)
        }
        Log.v(tag, "Client Disconnected, VPN is up")
        return super.onUnbind(intent)
    }

    override fun onDestroy() {
        // Note: This might not get called (depending on how it got invoked)
        // it for granted all exits will be here.
        Log.v(tag, "Service got Destroyed")
        super.onDestroy()
    }

    /**
     * EntryPoint for the Service, gets Called when AndroidController.cpp calles bindService.
     * Returns the [VPNServiceBinder] so QT can send Requests to it.
     */
    override fun onBind(intent: Intent?): IBinder? {
        Log.v(tag, "Got Bind request")
        init()
        if (mNotificationHandler.needsNotificationPermission()) {
            mBinder.dispatchEvent(CoreBinder.EVENTS.requestNotificationPermission)
        }
        return mBinder
    }

    /**
     * Might be the entryPoint if the Service gets Started via an Service Intent: Might be from
     * Always-On-Vpn from Settings or from Booting the device and having "connect on boot" enabled.
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.i(tag, "Service Started by Intent")
        init()
        if (isUp) {
            // In case a user has "always-on" and "start-on-boot" enabled, we might
            // get this multiple times.
            return START_NOT_STICKY
        }
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
                Log.e(tag, "VPN service was triggered without defining a Server or having a tunnel")
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
        Log.i(tag, "OS Revoked VPN permission")
        this.turnOff()
        super.onRevoke()
    }

    var connectionTime: Long = 0
        get() {
            return mConnectionTime
        }

    /**
     * Checks if there is a config loaded or some available in the Storage to fetch. if this is
     * false calling {reconnect()} will abort.
     * @returns whether a config is found.
     */
    var canActivate: Boolean = false
        get() {
            if (mConfig != null) {
                return true
            }
            val lastConfString = Prefs.get(this).getString("lastConf", "")
            return !lastConfString.isNullOrEmpty()
        }
    var cityname: String = ""
        get() {
            return mCityname
        }

    var isUp: Boolean = false
        get() {
            return currentTunnelHandle >= 0
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
    fun checkPermissions(): Intent? {
        // See https://developer.android.com/guide/topics/connectivity/vpn#connect_a_service
        // Call Prepare, if we get an Intent back, we dont have the VPN Permission
        // from the user. So we need to pass this to our main Activity and exit here.
        val intent = prepare(this)
        return intent
    }

    fun turnOn(json: JSONObject?, useFallbackServer: Boolean = false, source: String? = null, isDaemonChangingServers: Boolean = false) {
        if (json == null) {
            throw Error("no json config provided")
        }
        Log.sensitive(tag, json.toString())
        val jServer: JSONObject = getNextServerConfig(json, useFallbackServer)
        val wireguard_conf = buildWireguardConfig(jServer, json)
        val wgConfig: String = wireguard_conf.toWgUserspaceString()
        if (wgConfig.isEmpty()) {
            throw Error("WG_Userspace config is empty, can't continue")
        }
        mCityname = json.getString("city")

        if (checkPermissions() != null) {
            throw Error("turn on was called without vpn-permission!")
        }

        val builder = Builder()
        setupBuilder(wireguard_conf, builder)
        builder.setSession("mvpn0")

        if (json.getBoolean("isOnboarding")) {
            Log.i(tag, "Finishing onboarding... do not turn on the VPN after gaining permission")
            mBinder.dispatchEvent(CoreBinder.EVENTS.onboardingCompleted, "")
            return
        }

        builder.establish().use { tun ->
            if (tun == null) {
                Log.e(tag, "Activation Error: did not get a TUN handle")
                return
            }
            // We should have everything to establish a new connection, turn down the old tunnel
            // now.
            if (currentTunnelHandle != -1) {
                Log.i(tag, "Currently have a connection, close old handle")
                // Record data transfer metrics, as we're switching servers/configs and this data will get reset
                // Data metrics must be recorded prior to wgTurnOff, or we can't get data from the config
                if (isSuperDooperMetricsActive) {
                    recordDataTransferMetrics()
                }

                WireGuardGo.wgTurnOff(currentTunnelHandle)
            }
            currentTunnelHandle = WireGuardGo.wgTurnOn("mvpn0", tun.detachFd(), wgConfig)
        }
        if (currentTunnelHandle < 0) {
            throw Error("Activation Error Wireguard-Error -> $currentTunnelHandle")
        } else {
            Log.i(tag, "Updated tunnel handle to: " + currentTunnelHandle)
        }
        protect(WireGuardGo.wgGetSocketV4(currentTunnelHandle))
        protect(WireGuardGo.wgGetSocketV6(currentTunnelHandle))

        mConfig = json
        // We don't want to record start metrics in several situations:
        // - If Super Dooper feature is not active.
        // - If this is an app-caused server switch.
        // - If this is a daemon-caused server switch.
        // This must be calculated after mConfig is set (on line above)
        val isChangingServers = isDaemonChangingServers || isAppChangingServers
        val shouldSkipStartTelemetry = !isSuperDooperMetricsActive || isChangingServers
        Log.i(tag, "Is changing server: " + shouldSkipStartTelemetry)

        // Store the config in case the service gets
        // asked boot vpn from the OS
        val prefs = Prefs.get(this)
        prefs.edit().putString("lastConf", json.toString()).apply()

        // Go foreground
        CannedNotification(mConfig)?.let { mNotificationHandler.show(it) }

        // Select a fallback IP address to use for connection health checks. If not on first server,
        // use the first server. If on first server and another server exists, use the second server.
        var fallbackIpv4 = ""
        if (currentServerConfig == 0) {
            if (json.getJSONArray("servers").length() >= 2) {
                fallbackIpv4 = json.getJSONArray("servers").getJSONObject(1).getString("ipv4AddrIn")
            }
        } else {
            fallbackIpv4 = json.getJSONArray("servers").getJSONObject(0).getString("ipv4AddrIn")
        }

        // If changing server, connection health is still running
        if (!isChangingServers) {
            mConnectionHealth.start(
                jServer.getString("ipv4AddrIn"),
                jServer.getString("ipv4Gateway"),
                json.getString("dns"),
                fallbackIpv4,
                isSuperDooperMetricsActive,
            )
        }

        // For `isGleanDebugTagActive` and `isSuperDooperMetricsActive` to work,
        // this must be after the mConfig is set to the latest data.
        val gleanTag = gleanDebugTag
        if (gleanTag != null) {
            Log.i(tag, "Setting Glean debug tag for daemon.")
            Glean.setDebugViewTag(gleanTag)
        }
        if (!shouldSkipStartTelemetry) {
            Log.v(tag, "Recording start telmetry")
            val installationIdString = json.getString("installationId")
            installationIdString?.let {
                try {
                    val installationId = UUID.fromString(installationIdString)
                    Session.installationId.set(installationId)
                } catch (e: Exception) {
                    Log.e(tag, "Daemon installation ID string was not UUID:")
                    Log.e(tag, e.toString())
                }
            }
            Pings.daemonsession.submit(
                Pings.daemonsessionReasonCodes.daemonFlush,
            )

            Session.daemonSessionStart.set()
            Session.daemonSessionId.generateAndSet()
            if (source != null) {
                Session.daemonSessionSource.set(source)
            }
            Pings.daemonsession.submit(
                Pings.daemonsessionReasonCodes.daemonStart,
            )
        } else {
            Log.v(tag, "Skipping recording of start telemetry")
        }
        mMetricsTimer.cancel() // if this is a server switch, time is already running and must be reset
        mMetricsTimer.start()
    }

    fun reconnect(forceFallBack: Boolean = false, source: String? = null) {
        // Save the current timestamp - so that a silent switch won't
        // reset the timer in the app.
        val currentConnectionTime = mConnectionTime

        val config =
            if (this.mConfig != null) {
                this.mConfig
            } else {
                val prefs = Prefs.get(this)
                val lastConfString = prefs.getString("lastConf", "")
                if (lastConfString.isNullOrEmpty()) {
                    // We have nothing to connect to -> Exit
                    Log.e(
                        tag,
                        "VPN service was triggered without defining a Server or having a tunnel",
                    )
                    throw Error("no config to use")
                }
                JSONObject(lastConfString)
            }

        Log.v(tag, "Try to reconnect tunnel with same conf")
        try {
            // When `reconnect` is called with `source = system`, it is a new activation.
            // In all other cases - where source is null - this is called from a failed health
            // check, and we're doing a silent server switch - which shouldn't record telemetry
            // for a new session.
            val shouldSkipMetrics = (source == null)
            this.turnOn(config, forceFallBack, source, shouldSkipMetrics)
        } catch (e: Exception) {
            Log.e(tag, "VPN service - Reconnect failed")
            // TODO: If we end up here, we might have screwed up the connection.
            // we should put out a notification that the user go into the app and does a manual
            // connection.
        }
        if (currentConnectionTime != 0.toLong()) {
            // In case we have had a connection timestamp,
            // restore that, so that the silent switch is not
            // putting people off. :)
            mConnectionTime = currentConnectionTime
        }
    }
    fun clearConfig() {
        Prefs.get(this).edit().apply() { putString("lastConf", "") }.apply()
        mConfig = null
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        // turnOff is not called when switching locations, doing a silent server switch from app, or doing a silent server switch from daemon...
        // Thus we always want to record end-of-session metrics here, and send the ping below.
        if (isSuperDooperMetricsActive) {
            // Data metrics must be recorded prior to wgTurnOff, or we can't get data from the config
            recordDataTransferMetrics()
        }

        WireGuardGo.wgTurnOff(currentTunnelHandle)
        currentTunnelHandle = -1
        // If the client is "dead", on a disconnect the
        // message won't be updated to 'you disconnected from X'
        // so we should get rid of it. :)
        val shouldClearNotification = !mBinder.isClientAttached
        stopForeground(shouldClearNotification)
        mConnectionHealth.stop()
        // Clear the notification message, so the content
        // is not "disconnected" in case we connect from a non-client.
        CannedNotification(mConfig)?.let { mNotificationHandler.hide(it) }
        if (isSuperDooperMetricsActive) {
            Log.v(tag, "Sending ending metrics")
            Session.daemonSessionEnd.set()
            Pings.daemonsession.submit(
                Pings.daemonsessionReasonCodes.daemonEnd,
            )

            // We are rotating the UUID here as a safety measure. It is rotated
            // again before the next session start, and we expect to see the
            // UUID created here in only one ping: The daemon ping with a
            // "flush" reason, which should contain this UUID and no other
            // metrics.
            Session.daemonSessionId.generateAndSet()
        } else {
            Log.v(tag, "Skipping ending metrics")
        }
        mMetricsTimer.cancel()
    }

    /** Configures an Android VPN Service Tunnel with a given Wireguard Config */
    private fun setupBuilder(config: Config, builder: Builder) {
        // Setup Split tunnel
        for (
            excludedApplication in
            config.`interface`.excludedApplications
            ) builder.addDisallowedApplication(
                excludedApplication,
            )

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

    /** Gets config value for {key} from the Current running Wireguard tunnel */
    private fun getConfigValue(key: String): String? {
        if (!isUp) {
            return null
        }
        val config = WireGuardGo.wgGetConfig(currentTunnelHandle) ?: return null
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
     * Create a Wireguard [Config] from a [json] string - The [json] will be created in
     * AndroidController.cpp
     */
    private fun buildWireguardConfig(jServer: JSONObject, obj: JSONObject): Config {
        val confBuilder = Config.Builder()

        val peerBuilder = Peer.Builder()
        val ep =
            InetEndpoint.parse(
                jServer.getString("ipv4AddrIn") + ":" + jServer.getString("port"),
            )
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

        // Select the best DNS - if user is using a special privacy or user-specified DNS, use that.
        // Otherwise, use the gateway for this server.
        val mainGatewayAddress = obj.getJSONArray("servers").getJSONObject(0).getString("ipv4AddrIn")
        val mainDnsAddress = obj.getString("dns")
        val isSpecialDNS = mainGatewayAddress != mainDnsAddress
        val dnsAddress = if (isSpecialDNS) {
            mainDnsAddress
        } else {
            jServer.getString("ipv4Gateway")
        }
        ifaceBuilder.addDnsServer(InetNetwork.parse(dnsAddress).address)

        val jExcludedApplication = obj.getJSONArray("excludedApps")
        (0 until jExcludedApplication.length()).toList().forEach {
            val appName = jExcludedApplication.get(it).toString()
            ifaceBuilder.excludeApplication(appName)
        }
        confBuilder.setInterface(ifaceBuilder.build())
        return confBuilder.build()
    }

    private fun getNextServerConfig(obj: JSONObject, useFallbackServer: Boolean): JSONObject {
        if (!useFallbackServer) {
            currentServerConfig = 0
        } else {
            // Potential future work: Use server weights when selecting next server here and in iOS.
            val numServers = obj.getJSONArray("servers").length()
            currentServerConfig = (currentServerConfig + 1) % numServers
        }
        return obj.getJSONArray("servers").getJSONObject(currentServerConfig)
    }

    fun setGleanUploadEnabled(uploadEnabled: Boolean) {
        Log.i(tag, "Setting glean upload enabled state: $uploadEnabled")

        Prefs.get(this).edit().apply {
            putBoolean("glean_enabled", uploadEnabled)
            apply()
        }

        Glean.setUploadEnabled(uploadEnabled)
    }

    private fun initializeGlean(uploadEnabled: Boolean) {
        val customDataPath = File(applicationContext.applicationInfo.dataDir, GLEAN_DATA_DIR).path
        val channel =
            if (this.packageName.endsWith(".debug")) {
                "staging"
            } else {
                "production"
            }

        Glean.registerPings(Pings)
        Glean.initialize(
            applicationContext = this.applicationContext,
            uploadEnabled = uploadEnabled,
            // GleanBuildInfo can only be generated for application,
            // We are in a library so we have to build it ourselves.
            buildInfo =
            BuildInfo(
                BuildConfig.VERSIONCODE,
                BuildConfig.SHORTVERSION,
                Calendar.getInstance(),
            ),
            configuration =
            Configuration(
                channel = channel,
                // When initializing Glean from outside the main process,
                // we need to provide it with a dataPath manually.
                dataPath = customDataPath,
            ),
        )

        Log.i(tag, "Initialized Glean for daemon. Upload enabled state: $uploadEnabled")
    }

    private fun recordDataTransferMetrics() {
        Log.v(tag, "Recording data metrics")
        val isServerInCountry = isServerLocatedInUserCountry // with an open getter, can't directly use isServerLocatedInUserCountry in smart cast
        if (isServerInCountry != null) {
            Session.serverInSameCountry.set(isServerInCountry)
        }
        val rxBytes = getConfigValue("rx_bytes")?.toLong()
        val txBytes = getConfigValue("tx_bytes")?.toLong()
        if (rxBytes != null && txBytes != null) {
            ConnectionHealth.dataTransferredRx.accumulateSingleSample(rxBytes)
            ConnectionHealth.dataTransferredTx.accumulateSingleSample(txBytes)
        } else {
            Log.e(tag, "Config gave bad value for rxBytes and/or txBytes")
        }
    }

    companion object {
        // This value cannot be "glean_data",
        // because that is the data path for the Glean data on the main application.
        // See:
        // https://mozilla.github.io/glean/book/reference/general/initializing.html#configuration
        internal const val GLEAN_DATA_DIR: String = "glean_daemon_data"

        @JvmStatic
        fun startService(c: Context) {
            c.applicationContext.startService(
                Intent(c.applicationContext, VPNService::class.java).apply {
                    putExtra("startOnly", true)
                },
            )
        }
    }
}
