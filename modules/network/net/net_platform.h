#ifndef MOD_NET_PLATFORM_H
#define MOD_NET_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOD_NET_MAX_IP_STRING 40
#define MOD_NET_MAX_MAC_STRING 18
#define MOD_NET_MAX_SSID_STRING 33
#define MOD_NET_MAX_DNS_COUNT 4

typedef enum {
        kModNetStatusOK = 0,
        kModNetStatusNotAvailable,
        kModNetStatusError,
} modNetStatus;

typedef struct {
        char value[MOD_NET_MAX_IP_STRING];
} modNetIPString;

typedef struct {
        char value[MOD_NET_MAX_MAC_STRING];
} modNetMACString;

typedef struct {
        char value[MOD_NET_MAX_SSID_STRING];
} modNetSSIDString;

typedef struct {
        int32_t value;
        bool    hasValue;
} modNetIntegerValue;

typedef struct {
        uint8_t count;
        char    addresses[MOD_NET_MAX_DNS_COUNT][MOD_NET_MAX_IP_STRING];
} modNetDNSList;

modNetStatus modNetPlatformGetIP(const char *interfaceSpecifier, modNetIPString *ip);
modNetStatus modNetPlatformGetMAC(const char *interfaceSpecifier, modNetMACString *mac);
modNetStatus modNetPlatformGetSSID(modNetSSIDString *ssid);
modNetStatus modNetPlatformGetBSSID(modNetMACString *bssid);
modNetStatus modNetPlatformGetRSSI(modNetIntegerValue *rssi);
modNetStatus modNetPlatformGetChannel(modNetIntegerValue *channel);
modNetStatus modNetPlatformGetDNS(modNetDNSList *dnsList);

#ifdef __cplusplus
}
#endif

#endif
