package com.mozilla.vpn

import android.net.VpnService
import android.os.Binder
import android.os.Parcel
import android.util.Log
import com.wireguard.config.*
import com.wireguard.crypto.Key
import org.json.JSONObject
import org.qtproject.qt5.android.bindings.QtActivity
import org.qtproject.qt5.android.bindings.QtActivityLoader
import org.qtproject.qt5.android.bindings.QtApplication


object ACTIONS{
    val initialize = 0;
    val activate = 1;
    val deactivate = 2;
    val checkStatus = 3;
    val getBackendLogs = 4;
    val emitConnected = 5;
    val emitDisconnected = 6;
}

public class VPNServiceBinder(service: VPNService): Binder() {

    val mService = service;
    val TAG = "VPNServiceBinder";

    override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
        Log.e(TAG, "GOT TRANSACTION $code");


        when(code){
            ACTIONS.activate -> {
                // Read JSON and Build Config Objects for the VPN
                val buffer = data.createByteArray();
                val jString = buffer?.let { String(it) };
                Log.d(TAG, jString);
                val obj = JSONObject(jString);

                // Build the Config for the new Tunnel
                val confBuilder = Config.Builder();

                //Add Peers (Servers)
                {   val jServer = obj.getJSONObject("server");
                    val peerBuilder = Peer.Builder();
                    val ep = InetEndpoint.parse(jServer.getString("ipv4AddrIn")+":"+jServer.getString("port"));
                    peerBuilder.setEndpoint(ep);
                    peerBuilder.setPublicKey(Key.fromBase64(jServer.getString("publicKey")))
                    //peerBuilder.setPreSharedKey(); // We dont have that.
                    // peerBuilder.setPersistentKeepalive(); //
                    // no idea.
                    val internet = InetNetwork.parse("0.0.0.0/0") // ALL THZE interwebs
                    peerBuilder.addAllowedIp(internet);
                    confBuilder.addPeer(peerBuilder.build())
                }();
                // Add the Device Interface
                {
                    val privateKey = obj.getJSONObject("keys").getString("privateKey");
                    val jDevice = obj.getJSONObject("device");
                    val ifaceBuilder = Interface.Builder();
                    ifaceBuilder.parsePrivateKey(privateKey);
                    ifaceBuilder.addAddress(InetNetwork.parse(jDevice.getString("ipv4Address")));
                    ifaceBuilder.addAddress(InetNetwork.parse(jDevice.getString("ipv6Address")));
                    confBuilder.setInterface(ifaceBuilder.build());
                }();

                // Try to Turn on the Tunnel.
                // Reply with QVariant<Bool> if it was sucsessfull.
                val conf = confBuilder.build();
                this.mService.createTunnel(confBuilder.build());
                if(this.mService.turnOn()){
                    reply?.writeByteArray(byteArrayOf(1));
                    return true;
                }
                reply?.writeByteArray(byteArrayOf(0));
                return false;
            }

            ACTIONS.deactivate -> {
                this.mService.turnOff();
                reply?.writeByteArray(byteArrayOf(1, 2, 3));
                return true;
            }
            else->{
                // Unknown Code
                Log.e(TAG, "Unknown Transaction Code $code");
            }

        }




        return super.onTransact(code, data, reply, flags)
    }
}