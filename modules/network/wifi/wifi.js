/*
 * Copyright (c) 2016-2024  Moddable Tech, Inc.
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

/*
        wifi
*/

import {isWasmEnvironment, requireCapability, callCapabilityMethod} from "base/wasm/host";

const WiFiModeValues = Object.freeze({
        // off: -5,                     // reserved. ESP32-only now. Could be extended to others.
        none: 0,
        station: 1,
        accessPoint: 2
        // 3 reserved for station & accessPoint (WiFi.mode.station | WiFi.mode.accessPoint)
});

const WiFiEvents = {
        gotIP: "gotIP",
        lostIP: "lostIP",
        connected: "connect",
        disconnected: "disconnect"
};

class XSWiFi @ "xs_wifi_destructor" {
        constructor(dictionary, onNotify) @ "xs_wifi_constructor";
        close() @ "xs_wifi_close";
        set onNotify() @ "xs_wifi_set_onNotify";

        build(dictionary, onNotify) {
                if (dictionary)
                        XSWiFi.connect(dictionary);
                if (onNotify)
                        this.onNotify = onNotify;
        }

        static set mode() @ "xs_wifi_set_mode";
        static get mode() @ "xs_wifi_get_mode";
        static scan(dictionary, callback) @ "xs_wifi_scan";
        static connect(dictionary) @ "xs_wifi_connect";         // no arguments to disconnect
        static accessPoint(dictionary) @ "xs_wifi_accessPoint";
        static close() {throw new Error("use WiFi.disconnect()")}               // deprecated
        static disconnect() {XSWiFi.connect();}
}

XSWiFi.Mode = WiFiModeValues;
XSWiFi.gotIP = WiFiEvents.gotIP;
XSWiFi.lostIP = WiFiEvents.lostIP;
XSWiFi.connected = WiFiEvents.connected;
XSWiFi.disconnected = WiFiEvents.disconnected;

class WasmWiFi {
        #handle;
        #capability;
        #onNotify;

        constructor(dictionary, onNotify) {
                this.#capability = requireCapability("wifi");
                const create = this.#capability.create ?? this.#capability.open ?? this.#capability.construct;
                if ("function" !== typeof create)
                        throw new Error("WASM Wi-Fi host must provide a create/open/construct function");
                this.#handle = create(dictionary ?? null, onNotify ?? null);
                if (onNotify)
                        this.onNotify = onNotify;
        }

        close() {
                if (!this.#handle)
                        return;
                const close = this.#capability.close;
                if ("function" === typeof close)
                        close(this.#handle);
                this.#handle = null;
        }

        set onNotify(callback) {
                this.#onNotify = callback ?? null;
                callCapabilityMethod("wifi", "setOnNotify", this.#handle, callback ?? null);
        }
        get onNotify() {
                return this.#onNotify ?? undefined;
        }

        build(dictionary, onNotify) {
                if (dictionary)
                        WasmWiFi.connect(dictionary);
                if (onNotify)
                        this.onNotify = onNotify;
        }

        static set mode(value) {
                callCapabilityMethod("wifi", "setMode", value);
        }
        static get mode() {
                return callCapabilityMethod("wifi", "getMode");
        }
        static scan(dictionary, callback) {
                return callCapabilityMethod("wifi", "scan", dictionary ?? null, callback ?? null);
        }
        static connect(dictionary) {
                return callCapabilityMethod("wifi", "connect", dictionary ?? null);
        }
        static accessPoint(dictionary) {
                return callCapabilityMethod("wifi", "accessPoint", dictionary ?? null);
        }
        static close() {
                throw new Error("use WiFi.disconnect()");
        }
        static disconnect() {
                return WasmWiFi.connect();
        }
}

WasmWiFi.Mode = WiFiModeValues;
WasmWiFi.gotIP = WiFiEvents.gotIP;
WasmWiFi.lostIP = WiFiEvents.lostIP;
WasmWiFi.connected = WiFiEvents.connected;
WasmWiFi.disconnected = WiFiEvents.disconnected;

export default class WiFi {
        constructor(...args) {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return new Implementation(...args);
        }

        static set mode(value) {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                Implementation.mode = value;
        }
        static get mode() {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return Implementation.mode;
        }
        static scan(...args) {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return Implementation.scan(...args);
        }
        static connect(...args) {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return Implementation.connect(...args);
        }
        static accessPoint(...args) {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return Implementation.accessPoint(...args);
        }
        static close() {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return Implementation.close();
        }
        static disconnect(...args) {
                const Implementation = isWasmEnvironment() ? WasmWiFi : XSWiFi;
                return Implementation.disconnect(...args);
        }
}

WiFi.Mode = WiFiModeValues;
WiFi.gotIP = WiFiEvents.gotIP;
WiFi.lostIP = WiFiEvents.lostIP;
WiFi.connected = WiFiEvents.connected;
WiFi.disconnected = WiFiEvents.disconnected;
