package com.mozilla.vpn
import android.content.Context
import android.content.Intent
import android.os.IBinder
import android.util.Log
import androidx.core.app.ActivityCompat.startActivityForResult
import com.wireguard.android.backend.*
import com.wireguard.config.Config


public class VPNService :   android.net.VpnService(), ServiceProxy  {
    val TAG = "VPNService";
    var tunnel: Tunnel? = null
    var mBinder: VPNServiceBinder? = null;
    private val REQUEST_CODE_VPN_PERMISSION = 23491

    override fun onBind(intent: Intent?): IBinder? {
        if(mBinder == null){
            mBinder = VPNServiceBinder(this);
        }
        Log.v(TAG, "Got Bind request");
        return mBinder;
    }

    private val backend: VpnServiceBackend = VpnServiceBackend(
        object : VpnServiceBackend.VpnServiceDelegate {
            override fun protect(socket: Int): Boolean {
                return this@VPNService.protect(socket)
            }
        }
    )


    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if(mBinder == null){
            mBinder = VPNServiceBinder(this);
        }
        Log.v(TAG, "ON START COMMAND");
        return super.onStartCommand(intent, flags, startId)
    }

    override fun getStatistic(tunnel: Tunnel): Statistics {
        TODO("Not yet implemented")

    }

    public fun createTunnel(conf: Config){
        if(this.tunnel != null){
            // Use Exsisting tunnel
            return;
        }
        this.tunnel = Tunnel("myCoolTunnel", conf);
    }

    public fun turnOn() {
        val tunnel = this.tunnel ?: return
        val config = tunnel.config
        Log.v(TAG, "Try to enable tunnel");
        Builder()
            .applyConfig(config)
            .establish()
            ?.let {
                backend.tunnelUp(tunnel, it, config.toWgUserspaceString());
            }
    }
    public fun turnOff() {
        Log.v(TAG, "Try to disable tunnel");
        this.tunnel?.let { backend.tunnelDown(it) }
    }
    companion object {
        @JvmStatic
        fun getPermissionIntent(context: Context): Intent? {
            return prepare(context)
        }
    }
}
