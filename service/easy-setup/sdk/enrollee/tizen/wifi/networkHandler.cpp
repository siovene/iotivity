/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <wifi.h>
#include "logger.h"
#include <unistd.h>
#include "logger.h"
#include "networkHandler.h"
#define LOG_TAG "Tizen ES"

const char *g_ssid;
const char *g_pass;
char *g_ipaddress;
bool is_connected = false;
wifi_ap_h connected_wifi;
NetworkEventCallback g_cb;
static void activate_wifi();

static const char*
print_state(wifi_connection_state_e state)
{
    switch (state)
    {
        case WIFI_CONNECTION_STATE_DISCONNECTED:
            return "Disconnected";
        case WIFI_CONNECTION_STATE_ASSOCIATION:
            return "Association";
        case WIFI_CONNECTION_STATE_CONNECTED:
            return "Connected";
        case WIFI_CONNECTION_STATE_CONFIGURATION:
            return "Configuration";
    }
}

void __wifi_connected_cb(wifi_error_e error_code, void *user_data)
{
    OC_LOG(INFO,LOG_TAG,"#### __connected ");
    is_connected = true;
    wifi_ap_get_ip_address(connected_wifi, WIFI_ADDRESS_FAMILY_IPV4, &g_ipaddress);
    OC_LOG_V(INFO,LOG_TAG,"#### __connected, Ipaddress=%s", g_ipaddress);
    g_cb(ES_OK);

}

bool __wifi_found_ap_cb(wifi_ap_h ap, void *user_data)
{
    OC_LOG(INFO,LOG_TAG,"#### __wifi_found_ap_cb received ");

    int error_code = 0;
    char *ap_name = NULL;
    wifi_connection_state_e state;

    error_code = wifi_ap_get_essid(ap, &ap_name);
    if (error_code != WIFI_ERROR_NONE)
    {
        OC_LOG(ERROR,LOG_TAG,"#### Fail to get AP name.");

        return false;
    }
    error_code = wifi_ap_get_connection_state(ap, &state);
    if (error_code != WIFI_ERROR_NONE)
    {
        OC_LOG(ERROR,LOG_TAG,"#### Fail to get state.");

        return false;
    }
    OC_LOG_V(INFO,LOG_TAG,"#### AP name : %s, state : %s", ap_name, print_state(state));

    if (strcmp(ap_name, g_ssid) == 0)
    {
        OC_LOG(INFO,LOG_TAG,"#### network found");
        wifi_ap_set_passphrase(ap, g_pass);
        connected_wifi = ap;
        error_code = wifi_connect(ap, __wifi_connected_cb, NULL);
        OC_LOG_V(INFO,LOG_TAG,"Code=%d", error_code);
    }
    OC_LOG(INFO,LOG_TAG,"#### __wifi_found_ap_cb received ");
    return true;
}
void __scan_request_cb(wifi_error_e error_code, void *user_data)
{
    OC_LOG(INFO, LOG_TAG, "__scan_request_cb");
    int error_code1;
    error_code1 = wifi_foreach_found_aps(__wifi_found_ap_cb, NULL);
    if (error_code1 != WIFI_ERROR_NONE)
        OC_LOG(INFO,LOG_TAG,"#### Fail to scan");

    OC_LOG(INFO, LOG_TAG,"#### __scan_request_cb exit ");
}

static void __wifi_activated_cb(wifi_error_e result, void *user_data)
{
    OC_LOG(INFO, LOG_TAG, "__wifi_activated_cb");
    if (result == WIFI_ERROR_NONE)
    {
        OC_LOG(INFO,LOG_TAG,"#### Success to activate Wi-Fi device!");
    }
    wifi_scan(__scan_request_cb, NULL);

}
static void activate_wifi()
{
    int error_code;
    error_code = wifi_initialize();
    OC_LOG_V(INFO,LOG_TAG,"#### WIFI INITIALIZED WITH STATUS :%d", error_code);

    error_code = wifi_activate(__wifi_activated_cb, NULL);
    OC_LOG_V(INFO,LOG_TAG,"#### WIFI ACTIVATED WITH STATUS :%d", error_code);

    bool wifi_activated = false;
    wifi_is_activated(&wifi_activated);
    if (wifi_activated)
    {
        OC_LOG(INFO,LOG_TAG,"#### Success to get Wi-Fi device state.");
        int scan_result = wifi_scan(__scan_request_cb, NULL);
        OC_LOG_V(INFO,LOG_TAG,"#### Wifi scan result:%d", scan_result);
    }
    else
    {
        OC_LOG(ERROR,LOG_TAG, "#### Fail to get Wi-Fi device state.");
    }
}

static void start()
{

    OC_LOG(INFO, LOG_TAG, "START");
    activate_wifi();
}

ESResult ConnectToWiFiNetwork(const char *ssid, const char *pass, NetworkEventCallback cb)
{
    OC_LOG_V(INFO, LOG_TAG, "ConnectToWiFiNetwork %s %s",ssid,pass);
    g_pass = pass;
    g_ssid = ssid;
    g_cb = cb;
    start();
    //sleep(5000);
    //if (is_connected)
    return ES_NETWORKCONNECTED;
    //else
    //return ES_NETWORKNOTCONNECTED;
}

ESResult getCurrentNetworkInfo(OCConnectivityType targetType, NetworkInfo *info)
{
    if (targetType == CT_ADAPTER_IP)
    {
        info->type = CT_ADAPTER_IP;
        info->ipaddr = g_ipaddress;
        if (strlen(g_ssid) <= MAXSSIDLEN)
        {
            strcpy(info->ssid, g_ssid);
            return ES_OK;
        }
        else
        {
            return ES_ERROR;
        }
    }

    return ES_ERROR;
}
