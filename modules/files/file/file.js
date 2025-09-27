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
        file
*/

import {isWasmEnvironment, callCapabilityMethod} from "base/wasm/host";

class XSFile @ "xs_file_destructor" {
        constructor(dictionary) @ "xs_File";

        read(type, count) @ "xs_file_read";
        write(...items) @ "xs_file_write";

        close() @ "xs_file_close";

        get length() @ "xs_file_get_length";
        get position() @ "xs_file_get_position";
        set position() @ "xs_file_set_position";

        static delete(path) @ "xs_file_delete";
        static exists(path) @ "xs_file_exists";
        static rename(path, name) @ "xs_file_rename";
};
Object.freeze(XSFile.prototype);

class WasmFile {
        #handle;

        constructor(dictionary) {
                this.#handle = callCapabilityMethod("file", "open", dictionary ?? {});
        }

        read(type, count) {
                return callCapabilityMethod("file", "read", this.#handle, type, count);
        }
        write(...items) {
                return callCapabilityMethod("file", "write", this.#handle, ...items);
        }

        close() {
                if (!this.#handle)
                        return;
                callCapabilityMethod("file", "close", this.#handle);
                this.#handle = null;
        }

        get length() {
                return callCapabilityMethod("file", "getLength", this.#handle);
        }
        get position() {
                return callCapabilityMethod("file", "getPosition", this.#handle);
        }
        set position(value) {
                callCapabilityMethod("file", "setPosition", this.#handle, value);
        }

        static delete(path) {
                return callCapabilityMethod("file", "delete", path);
        }
        static exists(path) {
                return callCapabilityMethod("file", "exists", path);
        }
        static rename(path, name) {
                return callCapabilityMethod("file", "rename", path, name);
        }
}
Object.freeze(WasmFile.prototype);

class XSIterator @ "xs_file_iterator_destructor" {
        constructor(path) @ "xs_File_Iterator";
        next() @ "xs_file_iterator_next";

        [Symbol.iterator]() {
                return {
                        iterator: this,
                        next() {
                                const result = {value: this.iterator.next()};
                                result.done = undefined === result.value;
                                return result;
                        }
                };
        }
};

class WasmIterator {
        #handle;

        constructor(path) {
                this.#handle = callCapabilityMethod("file", "openIterator", path);
        }
        next() {
                return callCapabilityMethod("file", "iteratorNext", this.#handle);
        }

        [Symbol.iterator]() {
                return {
                        iterator: this,
                        next() {
                                const value = this.iterator.next();
                                return { value, done: undefined === value };
                        }
                };
        }
}
Object.freeze(WasmIterator.prototype);

class XSDirectory {
        static create(path) @ "xs_directory_create";
        static delete(path) @ "xs_directory_delete";
};

class WasmDirectory {
        static create(path) {
                return callCapabilityMethod("file", "directoryCreate", path);
        }
        static delete(path) {
                return callCapabilityMethod("file", "directoryDelete", path);
        }
}

class XSSystem {
        static config() @ "xs_file_system_config";
        static info() @ "xs_file_system_info";
};

class WasmSystem {
        static config() {
                return callCapabilityMethod("file", "systemConfig");
        }
        static info() {
                return callCapabilityMethod("file", "systemInfo");
        }
}

export class File {
        constructor(...args) {
                const Implementation = isWasmEnvironment() ? WasmFile : XSFile;
                return new Implementation(...args);
        }

        static delete(...args) {
                const Implementation = isWasmEnvironment() ? WasmFile : XSFile;
                return Implementation.delete(...args);
        }
        static exists(...args) {
                const Implementation = isWasmEnvironment() ? WasmFile : XSFile;
                return Implementation.exists(...args);
        }
        static rename(...args) {
                const Implementation = isWasmEnvironment() ? WasmFile : XSFile;
                return Implementation.rename(...args);
        }
}

export class Iterator {
        constructor(...args) {
                const Implementation = isWasmEnvironment() ? WasmIterator : XSIterator;
                return new Implementation(...args);
        }
}

export class Directory {
        static create(...args) {
                const Implementation = isWasmEnvironment() ? WasmDirectory : XSDirectory;
                return Implementation.create(...args);
        }
        static delete(...args) {
                const Implementation = isWasmEnvironment() ? WasmDirectory : XSDirectory;
                return Implementation.delete(...args);
        }
}

export class System {
        static config(...args) {
                const Implementation = isWasmEnvironment() ? WasmSystem : XSSystem;
                return Implementation.config(...args);
        }
        static info(...args) {
                const Implementation = isWasmEnvironment() ? WasmSystem : XSSystem;
                return Implementation.info(...args);
        }
}

export default Object.freeze({
        File, Iterator, System, Directory
});
