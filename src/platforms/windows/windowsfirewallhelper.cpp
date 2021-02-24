/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logger.h"
#include "leakdetector.h"
#include "windowsfirewallhelper.h"

#include <QObject>
#include <QNetworkInterface>
#include <windows.h>
#include <fwpmu.h>
#include <stdio.h>
#include <comdef.h>
#include <netfw.h>

#include <initguid.h>
#include <guiddef.h>


DEFINE_GUID(MOZILLA_BIND_IP_V4_GUID, 0x3dc73ff7,0xbf6c,0x443e,0xa7,0x67,0x00,0x92,0xbc,0x89,0x75,0x60);
DEFINE_GUID(MOZILLA_BIND_IP_SUBLAYER,0x9d83081d,0x32cc,0x42b3,0x86,0x73,0xcc,0x90,0x04,0x8d,0x59,0xbd);


namespace {
Logger logger(LOG_WINDOWS, "WindowsFirewallHelper");
WindowsFirewallHelper* s_instance= nullptr;

}
WindowsFirewallHelper* WindowsFirewallHelper::instance(){
    if(s_instance == nullptr){
        s_instance = new WindowsFirewallHelper();
    }
    return s_instance;
}

WindowsFirewallHelper::WindowsFirewallHelper(){
  MVPN_COUNT_CTOR(WindowsFirewallHelper);

  HANDLE    engineHandle = NULL;
  DWORD    result = ERROR_SUCCESS;
  // Use dynamic sessions for efficiency and safety:
  //  -> Filtering policy objects are deleted even when the application crashes.
  FWPM_SESSION0 session;
  memset(&session, 0, sizeof(session));
  session.flags = FWPM_SESSION_FLAG_DYNAMIC;

  logger.log() << ("Opening the filter engine.\n");

  result = FwpmEngineOpen0(
      NULL,
      RPC_C_AUTHN_WINNT,
      NULL,
      &session,
      &engineHandle );

  if (result != ERROR_SUCCESS){
      logger.log() << "FwpmEngineOpen0 failed. Return value:.\n" << result;
      return;
  }
  UuidCreate(&mLayerID);

  logger.log() << ("Filter engine opened successfully.\n");
  mSessionHandle = engineHandle;
  FWPM_SUBLAYER0 subLayer;

  memset(&subLayer, 0, sizeof(subLayer));
  subLayer.subLayerKey = mLayerID;
  subLayer.displayData.name = (PWSTR)L"MozillaVPN-PerAPPKEY";
  subLayer.weight = 0xFFFF;

  result = FwpmSubLayerAdd0(mSessionHandle, &subLayer, NULL);
  if (result != ERROR_SUCCESS){
         logger.log() << "FwpmSubLayerAdd0 failed. Return value:.\n" << result;
         return;
  }

}

WindowsFirewallHelper::~WindowsFirewallHelper() {
  MVPN_COUNT_DTOR(WindowsFirewallHelper);
}


bool WindowsFirewallHelper::excludeApp(QString path){
     int interface_index = 30;// QNetworkInterface::interfaceIndexFromName("FirefoxPrivateNetworkVPN");

     logger.log() << "FirefoxPrivateNetworkVPN has index " <<interface_index;

    /* Wireguard go sets 2 essential rules to an app:
     *  permitTunInterface() weight-> 12
     *  blockAll() weight-> 0
     *
     *  So to reverse this, we need to allow All trafic for the App with
     *  >= 0
     *  and disallow TUN with >=13
     */


    logger.log() << "Adding exlucde Rule for " << path;
    DWORD   result = ERROR_SUCCESS;

    // Get the AppID for the Executable;
    std::wstring wstr = path.toStdWString();
    PCWSTR appPath = wstr.c_str();
    FWP_BYTE_BLOB *appID = NULL;
    uint64_t filterID = 0;

    result = FwpmGetAppIdFromFileName0(appPath, &appID);
    if (result != ERROR_SUCCESS){
        logger.log() << "FwpmGetAppIdFromFileName0 failed. Return value:.\n" << result;
        return false;
    }
    // Condition 1: Request must come from the .exe
    FWPM_FILTER_CONDITION0 conds[2];
    conds[0].fieldKey = FWPM_CONDITION_ALE_APP_ID;
    conds[0].matchType = FWP_MATCH_EQUAL;
    conds[0].conditionValue.type = FWP_BYTE_BLOB_TYPE;
    conds[0].conditionValue.byteBlob = appID;

    // Condition 2: Request may NOT be targeting the TUN interface
    conds[1].fieldKey = FWPM_CONDITION_INTERFACE_INDEX;
    conds[1].matchType = FWP_MATCH_NOT_EQUAL;
    conds[1].conditionValue.type = FWP_UINT32 ;
    conds[1].conditionValue.uint32  = interface_index;


    // Assemble the Filter base
    FWPM_FILTER0 filter;
    memset(&filter, 0, sizeof(filter));
    filter.filterCondition = conds;
    filter.numFilterConditions = 2;
    filter.action.type = FWP_ACTION_PERMIT;
    filter.weight.type =FWP_UINT8;
    filter.weight.uint8 = 15;
    filter.subLayerKey= mLayerID;
    filter.flags = FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT; // Make this decission only blockable by veto

    // Build and add the Filters
    // #1 Permit outbound IPv4 traffic.
    {
       QString name = QString("Permit (out) IPv4 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // #2 Permit inbound IPv4 traffic.
    {
       QString name = QString("Permit (in)  IPv4 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // #3 Permit outbound IPv6 traffic.
    {
       QString name = QString("Permit (out) IPv6 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // #4 Permit inbound IPv6 traffic.
    {
       QString name = QString("Permit (in)  IPv6 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }

    //only apply following filters to the TUN interface
    // tldr; block all trafic on TUN for the .exe
    conds[1].matchType = FWP_MATCH_EQUAL;
    filter.action.type = FWP_ACTION_BLOCK;
    filter.weight.type =FWP_UINT8;
    filter.weight.uint8 = 15;

    // #5 Block outbound IPv4 traffic.
    {
       QString name = QString("Block TUN (out) IPv4 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // #6 Block inbound IPv4 traffic.
    {
       QString name = QString("Block TUN (in)  IPv4 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // #7 Block outbound IPv6 traffic.
    {
       QString name = QString("Block TUN (out) IPv6 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // #8 Block inbound IPv6 traffic.
    {
       QString name = QString("Block TUN (in)  IPv6 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }
    // Only check the First condition -> APPID Match
    filter.numFilterConditions = 1;
    // Make a Callout to rebind the open request to the Right Interface
    filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
    filter.action.calloutKey = MOZILLA_BIND_IP_V4_GUID;
    // #9 Rewrite ipv4 traffic
    {
       QString name = QString("Force eth0 for IPv4 Traffic of: "+path);
       std::wstring wname = name.toStdWString();
       PCWSTR filterName = wname.c_str();
       filter.displayData.name = (PWSTR)filterName;
       filter.layerKey = FWPM_LAYER_ALE_BIND_REDIRECT_V4;
       result = FwpmFilterAdd0(mSessionHandle, &filter, NULL, &filterID);

       if (result != ERROR_SUCCESS){
           logger.log() << "Failed to set rule "<< name <<"\n" << result;
           return false;
       }
    }

    logger.log() << "Ruleset applied for: "<< path;
    return true;
}
