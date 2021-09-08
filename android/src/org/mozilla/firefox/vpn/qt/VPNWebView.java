/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.RemoteException;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebSettings.PluginState;
import android.webkit.WebStorage;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.CookieManager;

import android.util.Log;

import com.android.installreferrer.api.InstallReferrerClient;
import com.android.installreferrer.api.InstallReferrerStateListener;
import com.android.installreferrer.api.ReferrerDetails;

import java.lang.Runnable;
import java.lang.String;
import java.util.concurrent.Semaphore;

public class VPNWebView
{
    private static final String TAG = "VPNWebView";

    private final Activity m_activity;
    private WebView m_webView = null;
    private InstallReferrerClient m_referrer;

    private native void nativeOnPageStarted(String url, Bitmap icon);
    private native void nativeOnError(int errorCode, String description, String url);

    private class VPNWebViewClient extends WebViewClient
    {
        VPNWebViewClient() { super(); }

        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url)
        {
            return false;
        }

        @Override
        public void onPageStarted(WebView view, String url, Bitmap favicon) {
          // While the login view is open, disable the ability to do screenshots.
          m_activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);

          super.onPageStarted(view, url, favicon);
          nativeOnPageStarted(url, favicon);
        }

        @Override
        public void onReceivedError(WebView view,
                                    int errorCode,
                                    String description,
                                    String url)
        {
            super.onReceivedError(view, errorCode, description, url);
            nativeOnError(errorCode, description, url);
        }
    }

    public VPNWebView(final Activity activity, final String userAgent)
    {
        Log.v(TAG, "created - userAgent: " + userAgent);

        m_activity = activity;

        m_referrer = InstallReferrerClient.newBuilder(activity).build();
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_webView = new WebView(m_activity);
                WebSettings webSettings = m_webView.getSettings();

                Log.e(TAG, "UA" + webSettings.getUserAgentString());

                webSettings.setAllowFileAccess(false);
                webSettings.setDatabaseEnabled(true);
                webSettings.setDomStorageEnabled(true);
                webSettings.setJavaScriptEnabled(true);
                webSettings.setGeolocationEnabled(false);
                webSettings.setBuiltInZoomControls(false);
                webSettings.setPluginState(PluginState.ON);

                m_webView.getSettings().setUserAgentString(userAgent);

                m_webView.setWebViewClient((WebViewClient)new VPNWebViewClient());
                sem.release();
            }
        });

        try {
            sem.acquire();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void setUrl(final String url)
    {
        Log.v(TAG, "load url: " + url);

        // Try to Get a Referrer and then Load the URL with it
        m_referrer.startConnection(new InstallReferrerStateListener() {
            @Override
            public void onInstallReferrerSetupFinished(int responseCode) {
                String referrerValue ="";
                if( responseCode == InstallReferrerClient.InstallReferrerResponse.OK){
                    try {
                        ReferrerDetails response =  m_referrer.getInstallReferrer();
                        referrerValue = "&" + response.getInstallReferrer();
                        Log.v(TAG, "Recived - referrer: " + referrerValue);

                    } catch (RemoteException e) {
                        Log.v(TAG, "Failed - referrer - " + e.toString());
                    }
                }else{
                    Log.v(TAG, "Failed - referrer not available ");
                }
                m_referrer.endConnection();

                // We now have a referrer - Load the URI
                final String refUrl = url + referrerValue;
                nativeOnPageStarted(refUrl, null);
                m_activity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() { m_webView.loadUrl(refUrl); }
                });
            }
            @Override
            public void onInstallReferrerServiceDisconnected() {}
        });
    }

    public WebView getWebView()
    {
       return m_webView;
    }

    public void destroy()
    {
        Log.v(TAG, "bye!");
        m_activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            m_webView.destroy();
            m_activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
          }
        });
    }
    public void clearStorage(){
        m_webView.clearCache(true);
        CookieManager.getInstance().removeAllCookies(null);
        CookieManager.getInstance().flush();
        WebStorage.getInstance().deleteAllData();
    }
}
