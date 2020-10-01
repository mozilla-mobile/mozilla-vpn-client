package com.mozilla.vpn;

import android.app.Notification;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

import com.wireguard.android.backend.TunnelManager;
import com.wireguard.android.backend.WireGuardVpnService;

import org.jetbrains.annotations.NotNull;

public class VPNService extends WireGuardVpnService {



    @NotNull
    @Override
    public TunnelManager<?> getTunnelManager() {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Toast.makeText(this, "service starting", Toast.LENGTH_SHORT).show();
        System.out.println("HELLO WORLD");
        // If we get killed, after returning from here, restart
        return START_STICKY;
    }

    public static void startQtAndroidService(Context context) {
        System.out.println("LETZ BOOT");
        context.startService(new Intent(context, VPNService.class));
    }

}