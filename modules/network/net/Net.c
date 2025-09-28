#include "xsmc.h"
#include "xsHost.h"

#include "net_platform.h"

static const char *xsOptionalInterface(xsMachine *the)
{
        if (xsmcArgc > 1 && xsmcTest(xsArg(1)))
                return xsmcToString(xsArg(1));
        return NULL;
}

void xs_net_get(xsMachine *the)
{
        const char *prop = xsmcToString(xsArg(0));
        const char *iface = xsOptionalInterface(the);

        if (0 == c_strcmp(prop, "IP")) {
                modNetIPString ip = {0};
                if (modNetPlatformGetIP(iface, &ip) == kModNetStatusOK && ip.value[0])
                        xsResult = xsString(ip.value);
        }
        else if (0 == c_strcmp(prop, "MAC")) {
                modNetMACString mac = {0};
                if (modNetPlatformGetMAC(iface, &mac) == kModNetStatusOK && mac.value[0])
                        xsResult = xsString(mac.value);
        }
        else if (0 == c_strcmp(prop, "SSID")) {
                modNetSSIDString ssid = {0};
                if (modNetPlatformGetSSID(&ssid) == kModNetStatusOK && ssid.value[0])
                        xsResult = xsString(ssid.value);
        }
        else if (0 == c_strcmp(prop, "BSSID")) {
                modNetMACString bssid = {0};
                if (modNetPlatformGetBSSID(&bssid) == kModNetStatusOK && bssid.value[0])
                        xsResult = xsString(bssid.value);
        }
        else if (0 == c_strcmp(prop, "RSSI")) {
                modNetIntegerValue rssi = {0};
                if (modNetPlatformGetRSSI(&rssi) == kModNetStatusOK && rssi.hasValue)
                        xsResult = xsInteger(rssi.value);
        }
        else if (0 == c_strcmp(prop, "CHANNEL")) {
                modNetIntegerValue channel = {0};
                if (modNetPlatformGetChannel(&channel) == kModNetStatusOK && channel.hasValue)
                        xsResult = xsInteger(channel.value);
        }
        else if (0 == c_strcmp(prop, "DNS")) {
                modNetDNSList list = {0};
                if (modNetPlatformGetDNS(&list) == kModNetStatusOK && list.count) {
                        xsResult = xsNewArray(list.count);
                        for (uint8_t i = 0; i < list.count; i++)
                                xsmcSetIndex(xsResult, i, xsString(list.addresses[i]));
                }
        }
}
