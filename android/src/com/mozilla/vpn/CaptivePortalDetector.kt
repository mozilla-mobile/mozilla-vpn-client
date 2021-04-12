/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn

import android.content.Context
import android.util.Log
import com.android.volley.Request
import com.android.volley.RequestQueue
import com.android.volley.Response
import com.android.volley.VolleyError
import com.android.volley.toolbox.StringRequest
import com.android.volley.toolbox.Volley
import org.json.JSONArray

class CaptivePortalDetector private constructor(c: Context) :
    Response.Listener<Boolean>,
    Response.ErrorListener {
    private val context: Context = c.applicationContext
    private var mIpv4List = JSONArray()
    private var mIpv6List = JSONArray()
    private var mEnabled = false;

    private var notificationMessage = "Guest Wi-Fi portal blocked"
    private var notificationHeader = "The guest Wi-Fi network you’re connected to requires action. Click to turn off VPN to see the portal."

    private val requestQueue: RequestQueue = Volley.newRequestQueue(context)
    private val tag = "CaptivePortalDetection"

    private var currentRequests = 0

    private val CAPTIVEPORTAL_HOST = "detectportal.firefox.com"


    fun saveCaptivePortalConfig(
        ipv4List: JSONArray,
        ipv6List: JSONArray,
        notificationHeader: String,
        notificationMessage: String
    ) {
        val prefs =
            context.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE)
        prefs.edit()
            .putString("ipv4List", ipv4List.toString())
            .putString("ipv6List", ipv6List.toString())
            .putString("captivePortalHeader", notificationHeader)
            .putString("captivePortalMessage", notificationMessage)
            .apply()

        mIpv4List = ipv4List
        mIpv6List = ipv6List
    }

    fun deactivate(){
        if(mEnabled){
            val prefs =
                context.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE)
            prefs.edit().putBoolean("captivePortal.Enabled",false).apply()
        }
        mEnabled= false;
    }

    fun readCaptivePortalConfig() {
        val prefs =
            context.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE)
        mEnabled = prefs.getBoolean("captivePortal.Enabled",false)
        mIpv4List = JSONArray(prefs.getString("ipv4List", "[]"))
        mIpv6List = JSONArray(prefs.getString("ipv6List", "[]"))
        notificationMessage = prefs.getString("captivePortalMessage", "").toString()
        notificationHeader = prefs.getString("captivePortalHeader", "").toString()
    }
    /* Tries to detect if a captive portal is present:
    * For all ip of [mIpv4List,mIpv6List] try to fetch a success.txt
    * If the content is not CAPTIVEPORTAL_REQUEST_CONTENT, report a captive portal
    * If all requests fail, nothing happens.
    * When a portal is detected we will show a notification that lets users disable vpn.
     */
    fun detectPortal() {
        Log.i(tag, "Start CaptivePortalScan")

        for (i in 0 until mIpv4List.length()) {
            val element = mIpv4List.get(i).toString()
            val url = "http://$element/success.txt"
            val request = CaptivePortalRequest(
                url, this, this
            )
            request.headers["host"] = CAPTIVEPORTAL_HOST;
            request.tag = tag
            requestQueue.add(request)
            currentRequests += 1

            Log.i(tag, "Started -> $url request")
        }
        for (i in 0 until mIpv6List.length()) {
            val element = mIpv6List.get(i).toString()
            val url = "http://[$element]/success.txt"
            Log.i(tag, url)
            val request = CaptivePortalRequest(
                url, this, this
            )
            request.headers["host"] = CAPTIVEPORTAL_HOST;
            request.tag = tag
            requestQueue.add(request)
            currentRequests += 1
        }
    }

    override fun onResponse(response: Boolean?) {
        if(!mEnabled){
            return;
        }
        currentRequests -= 1
        if(response == null){
            // This should not happen.
            return;
        }
        Log.i(tag, "Captive Portal request reports success")
        onResult(response)
    }

    override fun onErrorResponse(error: VolleyError?) {
        currentRequests -= 1
        error?.let {
            Log.e(tag, "Error -> -- ${it.message} ")
        }

        if (currentRequests <= 0) {
            Log.i(tag, "All Captive Portal requests failed")
            onResult(false)
        }
    }

    fun onResult(detected: Boolean) {
        // Cancel all outstanding requests
        requestQueue.cancelAll(tag)
        currentRequests = 0
        if (!detected) {
            return
        }
        if(!mEnabled){
            return;
        }

        val nt = NotificationUtil.get(context)
        nt.notifyCaptivePortal(notificationHeader, notificationMessage)
    }

    companion object {
        private var instance: CaptivePortalDetector? = null
        fun get(context: Context): CaptivePortalDetector {
            if (instance == null) {
                instance = CaptivePortalDetector(context)
                instance!!.readCaptivePortalConfig()
            }
            return instance as CaptivePortalDetector
        }
        fun get(): CaptivePortalDetector? {
            return instance
        }
    }
}
