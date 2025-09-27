/*
 * Copyright (c) 2016-2017  Moddable Tech, Inc.
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
        net
*/

import {isWasmEnvironment, callCapabilityMethod} from "base/wasm/host";

class XSNet {
        static get() @ "xs_net_get";
        static resolve(host, callback) @ "xs_net_resolve";
}

class WasmNet {
        static get() {
                return callCapabilityMethod("net", "get");
        }
        static resolve(host, callback) {
                return callCapabilityMethod("net", "resolve", host, callback);
        }
}

export default class Net {
        static get(...args) {
                return (isWasmEnvironment() ? WasmNet : XSNet).get(...args);
        }
        static resolve(...args) {
                return (isWasmEnvironment() ? WasmNet : XSNet).resolve(...args);
        }
}
