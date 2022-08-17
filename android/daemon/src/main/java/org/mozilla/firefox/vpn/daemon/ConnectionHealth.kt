package org.mozilla.firefox.vpn.daemon

import android.annotation.SuppressLint
import android.content.Context
import android.net.*
import android.os.Build
import android.os.CountDownTimer
import android.os.Handler
import androidx.annotation.RequiresApi
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors


@RequiresApi(Build.VERSION_CODES.R)
class ConnectionHealth (service: VPNService): ConnectivityDiagnosticsManager.ConnectivityDiagnosticsCallback(){
    private val mService: VPNService = service;
    private val PING_TIMEOUT = 3000 // ms

    private var mEndPoint : String =""
    private var mGateway : String =""

    var mActive = false;
    var mVPNNetwork:Network? = null;
    var mWorker = Executors.newSingleThreadExecutor();

    fun start(endpoint:String, gateway:String){
        if(mActive){
            return;
        }
        Log.e("BASTI", "REGISTERING")
        mEndPoint=endpoint;
        mGateway = gateway;


        val mConnectivityManager = mService.getSystemService(Context.CONNECTIVITY_SERVICE)
                as ConnectivityManager;
        mConnectivityManager.registerNetworkCallback(vpnNetworkRequest,networkCallbackHandler);
        mActive=true;
        mTaskTimer.start();
    }
    fun stop(){
        mActive=false;
        mTaskTimer.cancel()
    }


    private val connectionHealthTimerMSec: Long = 10000
    private val mTaskTimer = object : CountDownTimer(
        connectionHealthTimerMSec,
        connectionHealthTimerMSec / 4
    ) {
        override fun onTick(millisUntilFinished: Long) {}
        override fun onFinish() {
            if(!mActive){
                return;
            }
            mWorker.submit(Runnable {
                kotlin.run {
                    val mConnectivityManager = mService.getSystemService(Context.CONNECTIVITY_SERVICE)
                            as ConnectivityManager;
                    val currentNetwork = mVPNNetwork ?: return@Runnable
                    val networkCaps = mConnectivityManager.getNetworkCapabilities(currentNetwork);
                    if(networkCaps?.hasTransport(NetworkCapabilities.TRANSPORT_VPN) != true){
                        Log.e("BASTI", "ACTIVE NETWORK NOT VPN")
                        return@Runnable
                    }
                    Log.e("BASTI", "ACTIVE NETWORK IS VPN")
                    val canReachGateway = currentNetwork.getByName(mEndPoint).isReachable(PING_TIMEOUT)
                    if(canReachGateway){
                        Log.i("BASTI", "Connection is fine")
                        return@Runnable
                    }
                    val anyNetworkCanConnect = mConnectivityManager.allNetworks.map {
                           it.getByName(mEndPoint).isReachable(PING_TIMEOUT)
                    }.any{ it }

                    if(anyNetworkCanConnect){
                        Log.i("BASTI", "Connection is refreshable")
                        // We the server is online but the connection got stale, let's reconnect
                        mService.mainLooper.run {
                            // Silent server switch to the same server
                            mService.reconnect();
                        }
                    }
                    Log.i("BASTI", "Connection is deeply broken!")
                    // The We can't reach the Server endpoint on any server,
                    // So we can't escalate that.
                }
            })
            this.start();
        }
    }

    private val networkCallbackHandler = object : ConnectivityManager.NetworkCallback(){
        override fun onAvailable(network: Network) {
            // We're connected to the vpn, let's use that for
            // status reports
            mVPNNetwork = network;
            super.onAvailable(network)
        }
        override fun onLost(network: Network) {
            mVPNNetwork = null;
            super.onLost(network)
        }
        override fun onUnavailable() {
            mVPNNetwork = null;
            super.onUnavailable()
        }
    };


    private val vpnNetworkRequest: NetworkRequest by lazy {
        // Network requests are & - conditions, so let's only request for
        // networks that use a VPN transport type.
        NetworkRequest.Builder().apply {
            clearCapabilities()
            // There might be default's set, docs are not clear
            // So let's remove all that we don't need
            removeTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            removeTransportType(NetworkCapabilities.TRANSPORT_BLUETOOTH);
            removeTransportType(NetworkCapabilities.TRANSPORT_WIFI);
            removeTransportType(NetworkCapabilities.TRANSPORT_LOWPAN);
            removeTransportType(NetworkCapabilities.TRANSPORT_ETHERNET);
            removeTransportType(NetworkCapabilities.TRANSPORT_WIFI_AWARE);
            addTransportType(NetworkCapabilities.TRANSPORT_VPN)
        }.build()
    }
}