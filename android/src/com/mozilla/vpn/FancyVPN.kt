package com.mozilla.vpn
import android.app.Service
import android.content.Context
import android.content.Intent
import android.net.VpnService
import android.os.IBinder
import com.wireguard.android.backend.TunnelManager
import com.wireguard.android.backend.WireGuardVpnService
import com.wireguard.config.Config
import com.wireguard.crypto.Key
import com.wireguard.crypto.KeyFormatException


public class FancyVPN : WireGuardVpnService() {
    val tm: TunnelManager<*> = TunnelManager(FancyVPN::class.java)

    override val tunnelManager: TunnelManager<*>
        get() = tm

    override fun onBind(intent: Intent?): IBinder? {
        return super.onBind(intent)
        TODO("Not yet implemented")
    }


    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val x = 10
        return super.onStartCommand(intent, flags, startId)
    }

}