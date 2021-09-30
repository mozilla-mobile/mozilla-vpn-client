package com.adjust.sdk;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.IInterface;
import android.os.Looper;
import android.os.Parcel;
import android.os.RemoteException;

import java.io.IOException;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

public class GooglePlayServicesClient {
  public static final class GooglePlayServicesInfo {
    private final String gpsAdid;
    private final Boolean trackingEnabled;

    GooglePlayServicesInfo(String gpdAdid, Boolean trackingEnabled) {
      this.gpsAdid = gpdAdid;
      this.trackingEnabled = trackingEnabled;
    }

    public String getGpsAdid() {
      return this.gpsAdid;
    }

    public Boolean isTrackingEnabled() {
      return this.trackingEnabled;
    }
  }

  public static GooglePlayServicesInfo getGooglePlayServicesInfo(
      Context context, long timeoutMilliSec) throws Exception {
    if (Looper.myLooper() == Looper.getMainLooper()) {
      throw new IllegalStateException(
          "Google Play Services info can't be accessed from the main thread");
    }

    try {
      PackageManager pm = context.getPackageManager();
      pm.getPackageInfo("com.android.vending", 0);
    } catch (Exception e) {
      throw e;
    }

    GooglePlayServicesConnection connection = new GooglePlayServicesConnection(timeoutMilliSec);
    Intent intent = new Intent("com.google.android.gms.ads.identifier.service.START");
    intent.setPackage("com.google.android.gms");
    if (context.bindService(intent, connection, Context.BIND_AUTO_CREATE)) {
      try {
        GooglePlayServicesInterface gpsInterface =
            new GooglePlayServicesInterface(connection.getBinder());
        GooglePlayServicesInfo googlePlayServicesInfo = new GooglePlayServicesInfo(
            gpsInterface.getGpsAdid(), gpsInterface.getTrackingEnabled(true));
        return googlePlayServicesInfo;
      } catch (Exception exception) {
        throw exception;
      } finally {
        context.unbindService(connection);
      }
    }
    throw new IOException("Google Play connection failed");
  }

  private static final class GooglePlayServicesConnection implements ServiceConnection {
    long timeoutMilliSec;
    boolean retrieved = false;
    private final LinkedBlockingQueue<IBinder> queue = new LinkedBlockingQueue<IBinder>(1);

    public GooglePlayServicesConnection(long timeoutMilliSec) {
      this.timeoutMilliSec = timeoutMilliSec;
    }

    public void onServiceConnected(ComponentName name, IBinder service) {
      try {
        this.queue.put(service);
      } catch (InterruptedException localInterruptedException) {
      }
    }

    public void onServiceDisconnected(ComponentName name) {}

    public IBinder getBinder() throws InterruptedException {
      if (this.retrieved) {
        throw new IllegalStateException();
      }
      this.retrieved = true;
      return (IBinder) this.queue.poll(this.timeoutMilliSec, TimeUnit.MILLISECONDS);
    }
  }

  private static final class GooglePlayServicesInterface implements IInterface {
    private IBinder binder;

    public GooglePlayServicesInterface(IBinder pBinder) {
      binder = pBinder;
    }

    public IBinder asBinder() {
      return binder;
    }

    public String getGpsAdid() throws RemoteException {
      Parcel data = Parcel.obtain();
      Parcel reply = Parcel.obtain();
      String id;
      try {
        data.writeInterfaceToken(
            "com.google.android.gms.ads.identifier.internal.IAdvertisingIdService");
        binder.transact(1, data, reply, 0);
        reply.readException();
        id = reply.readString();
      } finally {
        reply.recycle();
        data.recycle();
      }
      return id;
    }

    public Boolean getTrackingEnabled(boolean paramBoolean) throws RemoteException {
      Parcel data = Parcel.obtain();
      Parcel reply = Parcel.obtain();
      Boolean limitAdTracking;
      try {
        data.writeInterfaceToken(
            "com.google.android.gms.ads.identifier.internal.IAdvertisingIdService");
        data.writeInt(paramBoolean ? 1 : 0);
        binder.transact(2, data, reply, 0);
        reply.readException();
        limitAdTracking = 0 != reply.readInt();
      } finally {
        reply.recycle();
        data.recycle();
      }
      return limitAdTracking != null ? !limitAdTracking : null;
    }
  }
}
