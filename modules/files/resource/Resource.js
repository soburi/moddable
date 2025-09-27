/*
 * Copyright (c) 2016-2019  Moddable Tech, Inc.
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

import {isWasmEnvironment, requireCapability, callCapabilityMethod} from "base/wasm/host";

class XSResource @ "Resource_destructor" {
     constructor(path) @ "Resource_constructor";
     static exists(path) @ "Resource_exists";
     get byteLength() @ "Resource_get_byteLength";
     slice(begin, end) @ "Resource_slice";
}

class WasmResource {
        #handle;
        #capability;

        constructor(path) {
                this.#capability = requireCapability("resource");
                const open = this.#capability.open ?? this.#capability.create ?? this.#capability.construct;
                if ("function" !== typeof open)
                        throw new Error("WASM resource host must provide an open/create/construct function");
                this.#handle = open(path);
        }

        static exists(path) {
                return callCapabilityMethod("resource", "exists", path);
        }

        get byteLength() {
                const fn = this.#capability.byteLength ?? this.#capability.getByteLength;
                if ("function" !== typeof fn)
                        throw new Error("WASM resource host missing byteLength/getByteLength");
                return fn.call(this.#capability, this.#handle);
        }

        slice(begin, end) {
                const fn = this.#capability.slice;
                if ("function" !== typeof fn)
                        throw new Error("WASM resource host missing slice method");
                return fn.call(this.#capability, this.#handle, begin, end);
        }
}

const Implementation = isWasmEnvironment() ? WasmResource : XSResource;

export default Implementation;
