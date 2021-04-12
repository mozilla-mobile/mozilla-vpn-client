package com.mozilla.vpn

import com.android.volley.*

class CaptivePortalRequest(
    url: String,
    responseListener: Response.Listener<Boolean>,
    errorListener: Response.ErrorListener
) : Request<Boolean>(Method.GET,url, errorListener) {

    private val CAPTIVEPORTAL_REQUEST_CONTENT = "success"

    private val listener = responseListener;
    override fun parseNetworkResponse(response: NetworkResponse?): Response<Boolean> {
        if(response == null){
            return Response.error(NetworkError())
        }
        // 1: If we get a redirect 100% Portal
        if(response.statusCode == 302){
            // A Redirect is defnitly a captive portal
            return Response.success(true, null)
        }
        // 2: Check if request ok && "success"
        if(response.statusCode == 200){
            // On 200 it needs to be "success" otherwise it's also a portal
            if(response.data.toString() ==  CAPTIVEPORTAL_REQUEST_CONTENT){
                return Response.success(false, null)
            }
            // If it's not success it's probably an xml repsonse telling us about the portal
            // assuming this is defnitly one.
            return Response.success(true, null)
        }
        return Response.error(ParseError(Exception("cant deciver if we have a portal")))
    }

    override fun deliverResponse(response: Boolean?) {
        listener.onResponse(response)
    }

}