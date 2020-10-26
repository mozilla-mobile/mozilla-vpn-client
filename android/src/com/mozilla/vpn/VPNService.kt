package com.mozilla.vpn
import android.content.Context
import android.content.Intent
import android.os.IBinder
import android.util.Log
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

    public fun turnOn(): Boolean {
        // See https://developer.android.com/guide/topics/connectivity/vpn#connect_a_service
        // Step 1 - Call Prepare and Fire Permission Intent if needed
        val intent = this.getPermissionIntent();
        if(intent == null){
            Log.e(TAG, "VPN Permission Already Present");
        }
        else{
            Log.e(TAG, "Requesting VPN Permission");
            this.startActivityForResult(intent);
            return false;
        };

        val tunnel = this.tunnel ?: return false

       tunnel.tunnelHandle?.let{
           this.protect(it);
       }

        val config = tunnel.config
        Log.v(TAG, "Try to turn on");
        val b = Builder().applyConfig(config);
        val file = b.establish();

        if(file != null){
                Log.v(TAG, "Got file Descriptor for VPN - Try to up");
                backend.tunnelUp(tunnel, file, config.toWgUserspaceString());
                return true;
        }
        Log.e(TAG, "Failed to get a File Descriptor for VPN");
        return false;
    }
    public fun turnOff() {
        Log.v(TAG, "Try to disable tunnel");
        this.tunnel?.let { backend.tunnelDown(it) }
    }
    fun getPermissionIntent(): Intent? {
        return prepare(this)
    }

    // Implemented in
    public external fun startActivityForResult(i: Intent);

}
