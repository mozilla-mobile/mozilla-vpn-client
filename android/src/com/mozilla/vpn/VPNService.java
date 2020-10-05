package com.mozilla.vpn;

import android.app.Notification;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

import com.wireguard.android.backend.TunnelManager;
import com.wireguard.android.backend.WireGuardVpnService;

import org.jetbrains.annotations.NotNull;

public class VPNService extends WireGuardVpnService {

    private  VPNService _this;



    @Override
    public void onCreate (){
        super.onCreate();
        _this = this;
    }

    private static TunnelManager<?> tm;
    @NotNull
    @Override
    public TunnelManager<?> getTunnelManager() {
        if(tm == null){
            tm = new TunnelManager(VPNService.class);
        }
        return tm;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Toast.makeText(this, "service starting", Toast.LENGTH_SHORT).show();
        System.out.println("HELLO WORLD");
        String command = intent.getExtras().get(EXTRA_COMMAND).toString();
        if(command == COMMAND_TURN_ON) {
            // Start showing a notification
        }else if( command == COMMAND_TURN_OFF){
            // Stop showing a Notification
        }
        // If we get killed, after returning from here, restart
        return super.onStartCommand(intent,flags,startId);
    }

    public static void startQtAndroidService(Context context) {
        System.out.println("LETZ BOOT");
        context.startService(new Intent(context, VPNService.class));
    }

}