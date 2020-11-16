/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.webkit.URLUtil;
import android.webkit.WebSettings;
import android.webkit.WebSettings.PluginState;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.util.Log;
import java.lang.Runnable;
import java.lang.String;
import java.util.concurrent.Semaphore;

public class VPNWebView
{
    private static final String TAG = "VPNWebView";

    private final Activity m_activity;
    private WebView m_webView = null;

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
        public void onPageStarted(WebView view, String url, Bitmap favicon)
        {
            Log.v(TAG, "Url changed: " + url);

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
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_webView = new WebView(m_activity);
                WebSettings webSettings = m_webView.getSettings();

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

        nativeOnPageStarted(url, null);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.loadUrl(url); }
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
            }
        });
    }
}
