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

import {isWasmEnvironment, requireCapability, callCapabilityMethod} from "base/wasm/host";

class XSServo @ "xs_servo_destructor" {
        constructor(dictionary) @ "xs_servo";
        close() @ "xs_servo_close";
        write() @ "xs_servo_write";
        writeMicroseconds() @ "xs_servo_writeMicroseconds";
}
Object.freeze(XSServo.prototype);

class WasmServo {
        #handle;
        #capability;

        constructor(dictionary) {
                this.#capability = requireCapability("servo");
                const open = this.#capability.open ?? this.#capability.create ?? this.#capability.construct;
                if ("function" !== typeof open)
                        throw new Error("WASM servo host must provide an open/create/construct function");
                this.#handle = open(dictionary ?? {});
        }
        close() {
                if (!this.#handle)
                        return;
                const close = this.#capability.close;
                if ("function" === typeof close)
                        close(this.#handle);
                this.#handle = null;
        }
        write(value) {
                callCapabilityMethod("servo", "write", this.#handle, value);
        }
        writeMicroseconds(value) {
                callCapabilityMethod("servo", "writeMicroseconds", this.#handle, value);
        }
}
Object.freeze(WasmServo.prototype);

export class Servo {
        constructor(...args) {
                const Implementation = isWasmEnvironment() ? WasmServo : XSServo;
                return new Implementation(...args);
        }
}
Object.freeze(Servo.prototype);

export default Servo;
