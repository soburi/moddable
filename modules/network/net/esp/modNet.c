/*
 * Copyright (c) 2016-2021 Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 * 
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "string.h"
#include "net_platform.h"

#if ESP32
	#include "esp_wifi.h"
#else
	#include "user_interface.h"
#endif

#include "lwip/err.h"
#include "lwip/dns.h"

void twoHex(uint8_t value, char *out)
{
	static const char *gHex = "0123456789ABCDEF";
	*out++ = espRead8(gHex + (value >> 4));
	*out++ = espRead8(gHex + (value & 15));
}

// platform selection helper reused across queries
#if ESP32
static esp_netif_t *
#else
static uint8_t
#endif
getNIF(const char *specifier)
{
	uint8_t wantsAP = 0, wantsStation = 0;

	if (specifier && specifier[0]) {
		const char *nif = specifier;
		wantsAP = 0 == strcmp(nif, "ap");
		wantsStation = 0 == strcmp(nif, "station");
#if ESP32
		if (0 == strcmp(nif, "ethernet"))
			return esp_netif_get_handle_from_ifkey("ETH_DEF");
		if (!wantsAP && !wantsStation) {	// if argument is IP address, find adapter that matches
			ip_addr_t dst;
			if (ipaddr_aton(nif, &dst)) {
				esp_netif_t *ifc = NULL;
				dst.u_addr.ip4.addr &= 0x00ffffff;	//@@ this only works for IPv4
				do {
					esp_netif_ip_info_t info = {0};
					if ((ESP_OK == esp_netif_get_ip_info(ifc, &info)) && ((info.ip.addr & 0x00ffffff) == dst.u_addr.ip4.addr))
						return ifc;
				} while (ifc != NULL);
			}
		}
#endif
	}

#if ESP32
	wifi_mode_t mode;
	esp_err_t err = esp_wifi_get_mode(&mode);

	if (err == ESP_ERR_WIFI_NOT_INIT) {
		esp_netif_t *eth = esp_netif_get_handle_from_ifkey("ETH_DEF");
		if (esp_netif_is_netif_up(eth))
			return eth;
	}

	if (err != ESP_OK)
		return NULL;

	if (wantsStation && ((WIFI_MODE_STA == mode) || (WIFI_MODE_APSTA == mode)))
		return esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
	if (wantsAP && ((WIFI_MODE_AP == mode) || (WIFI_MODE_APSTA == mode)))
		return esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
	if (wantsAP || wantsStation)
		return NULL;
	if (WIFI_MODE_AP == mode)
		return esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
	if (WIFI_MODE_STA == mode)
		return esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
	return NULL;
#else
	uint8 mode = wifi_get_opmode();
	if (wantsStation && ((STATION_MODE == mode) || (STATIONAP_MODE == mode)))
		return STATION_IF;
	if (wantsAP && ((SOFTAP_MODE == mode) || (STATIONAP_MODE == mode)))
		return SOFTAP_IF;
	if (wantsAP || wantsStation)
		return ~0;
	if (SOFTAP_MODE == mode)
		return SOFTAP_IF;
	if (STATION_MODE == mode)
		return STATION_IF;
	return 255;
#endif
}


modNetStatus modNetPlatformGetIP(const char *interfaceSpecifier, modNetIPString *ip)
{
#if ESP32
        esp_netif_ip_info_t info = {0};
        esp_netif_t *nif = getNIF(interfaceSpecifier);

        if (!nif)
                return kModNetStatusNotAvailable;

        if ((ESP_OK == esp_netif_get_ip_info(nif, &info)) && info.ip.addr) {
#if LWIP_IPV4 && LWIP_IPV6
                ip4_addr_t t = {info.ip.addr};
                ip4addr_ntoa_r(&t, ip->value, sizeof(ip->value));
#else
                ipaddr_ntoa_r(&info.ip, ip->value, sizeof(ip->value));
#endif
                return kModNetStatusOK;
        }
        return kModNetStatusNotAvailable;
#else
        struct ip_info info;
        uint8_t nif = getNIF(interfaceSpecifier);

        if (255 == nif)
                return kModNetStatusNotAvailable;

        if (wifi_get_ip_info(nif, &info) && (ip4_addr1(&info.ip) || ip4_addr2(&info.ip) || ip4_addr3(&info.ip) || ip4_addr4(&info.ip))) {
                ipaddr_ntoa_r(&info.ip, ip->value, sizeof(ip->value));
                return kModNetStatusOK;
        }
        return kModNetStatusNotAvailable;
#endif
}

modNetStatus modNetPlatformGetMAC(const char *interfaceSpecifier, modNetMACString *mac)
{
        uint8_t macaddr[6] = {0};
#if ESP32
        esp_netif_t *netif = getNIF(interfaceSpecifier);

        if (!netif)
                return kModNetStatusNotAvailable;

        if (ESP_OK != esp_netif_get_mac(netif, macaddr))
                return kModNetStatusNotAvailable;
#else
        uint8_t nif = getNIF(interfaceSpecifier);

        if (255 == nif)
                return kModNetStatusNotAvailable;

        if (!wifi_get_macaddr(nif, macaddr))
                return kModNetStatusNotAvailable;
#endif

        char *out = mac->value;
        twoHex(macaddr[0], out); out += 2; *out++ = ':';
        twoHex(macaddr[1], out); out += 2; *out++ = ':';
        twoHex(macaddr[2], out); out += 2; *out++ = ':';
        twoHex(macaddr[3], out); out += 2; *out++ = ':';
        twoHex(macaddr[4], out); out += 2; *out++ = ':';
        twoHex(macaddr[5], out); out += 2; *out++ = 0;
        return kModNetStatusOK;
}

modNetStatus modNetPlatformGetSSID(modNetSSIDString *ssid)
{
#if ESP32
        wifi_ap_record_t config;

        if ((ESP_OK == esp_wifi_sta_get_ap_info(&config)) && config.ssid[0]) {
                strncpy(ssid->value, (const char *)config.ssid, sizeof(ssid->value));
                ssid->value[sizeof(ssid->value) - 1] = 0;
                return kModNetStatusOK;
        }
#else
        struct station_config config;

        if (wifi_station_get_config(&config) && config.ssid[0]) {
                strncpy((char *)ssid->value, (const char *)config.ssid, sizeof(ssid->value));
                ssid->value[sizeof(ssid->value) - 1] = 0;
                return kModNetStatusOK;
        }
#endif
        return kModNetStatusNotAvailable;
}

modNetStatus modNetPlatformGetBSSID(modNetMACString *bssid)
{
#if ESP32
        wifi_ap_record_t config;

        if (ESP_OK == esp_wifi_sta_get_ap_info(&config)) {
                char *out = bssid->value;
                twoHex(config.bssid[0], out); out += 2; *out++ = ':';
                twoHex(config.bssid[1], out); out += 2; *out++ = ':';
                twoHex(config.bssid[2], out); out += 2; *out++ = ':';
                twoHex(config.bssid[3], out); out += 2; *out++ = ':';
                twoHex(config.bssid[4], out); out += 2; *out++ = ':';
                twoHex(config.bssid[5], out); out += 2; *out++ = 0;
                return kModNetStatusOK;
        }
#else
        struct station_config config;

        if (wifi_station_get_config(&config)) {
                char *out = bssid->value;
                twoHex(config.bssid[0], out); out += 2; *out++ = ':';
                twoHex(config.bssid[1], out); out += 2; *out++ = ':';
                twoHex(config.bssid[2], out); out += 2; *out++ = ':';
                twoHex(config.bssid[3], out); out += 2; *out++ = ':';
                twoHex(config.bssid[4], out); out += 2; *out++ = ':';
                twoHex(config.bssid[5], out); out += 2; *out++ = 0;
                return kModNetStatusOK;
        }
#endif
        return kModNetStatusNotAvailable;
}

modNetStatus modNetPlatformGetRSSI(modNetIntegerValue *rssi)
{
#if ESP32
        wifi_ap_record_t config;

        if (ESP_OK == esp_wifi_sta_get_ap_info(&config)) {
                rssi->value = config.rssi;
                rssi->hasValue = true;
                return kModNetStatusOK;
        }
#else
        rssi->value = wifi_station_get_rssi();
        rssi->hasValue = true;
        return kModNetStatusOK;
#endif
        rssi->hasValue = false;
        return kModNetStatusNotAvailable;
}

modNetStatus modNetPlatformGetChannel(modNetIntegerValue *channel)
{
#if ESP32
        uint8_t primary;
        wifi_second_chan_t second;

        if (ESP_OK == esp_wifi_get_channel(&primary, &second)) {
                channel->value = primary;
                channel->hasValue = true;
                return kModNetStatusOK;
        }
#else
        channel->value = wifi_get_channel();
        channel->hasValue = true;
        return kModNetStatusOK;
#endif
        channel->hasValue = false;
        return kModNetStatusNotAvailable;
}

modNetStatus modNetPlatformGetDNS(modNetDNSList *dnsList)
{
        u8_t i = 0;

        dnsList->count = 0;
        do {
#if ESP32
                const ip_addr_t *addr = dns_getserver(i);
#else
                const ip_addr_t address = dns_getserver(i);
                const ip_addr_t *addr = &address;
#endif
#if LWIP_IPV4 && LWIP_IPV6
                if (!addr->u_addr.ip4.addr)
#else
                if (!addr->addr)
#endif
                        break;

                if (i >= MOD_NET_MAX_DNS_COUNT)
                        break;

                ipaddr_ntoa_r(addr, dnsList->addresses[dnsList->count], sizeof(dnsList->addresses[0]));
                dnsList->count += 1;
                i++;
        } while (true);

        return dnsList->count ? kModNetStatusOK : kModNetStatusNotAvailable;
}

