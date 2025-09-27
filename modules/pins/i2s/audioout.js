/*
 * Copyright (c) 2018-2021  Moddable Tech, Inc.
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

import {isWasmEnvironment, requireCapability} from "base/wasm/host";

class XSMixer @ "xs_audioout_destructor" {
        constructor(dictionary) @ "xs_audioout";
        close() @ "xs_audioout_close";
        enqueue(stream, kind, buffer, repeat, offset, count) @ "xs_audioout_enqueue";
        mix(samples) @ "xs_audioout_mix";
        length(stream) @ "xs_audioout_length";
        get sampleRate() @ "xs_audioout_get_sampleRate";
        get numChannels() @ "xs_audioout_get_numChannels";
        get bitsPerSample() @ "xs_audioout_get_bitsPerSample";
        get streams() @ "xs_audioout_get_streams";
}

function build(dictionary) @ "xs_audioout_build";

class XSAudioOut extends XSMixer {
        constructor(dictionary) {
                super(dictionary);
                build.call(this, dictionary);
        }
        start() @ "xs_audioout_start";
        stop() @ "xs_audioout_stop";
        get mix() {}
}

const CAPABILITY_NAME = "audioout";

function callAudio(methods, ...args) {
        const capability = requireCapability(CAPABILITY_NAME);
        const names = Array.isArray(methods) ? methods : [methods];
        for (const name of names) {
                const fn = capability?.[name];
                if ("function" === typeof fn)
                        return fn.apply(capability, args);
        }
        throw new Error(`WASM audio host missing method ${names.join("/")}`);
}

class WasmMixer {
        _handle;

        constructor(dictionary) {
                this._handle = callAudio(["createMixer", "openMixer", "open"], dictionary ?? {});
        }
        close() {
                if (!this._handle)
                        return;
                callAudio(["closeMixer", "close"], this._handle);
                this._handle = null;
        }
        enqueue(stream, kind, buffer, repeat, offset, count) {
                return callAudio("enqueue", this._handle, stream, kind, buffer, repeat, offset, count);
        }
        mix(samples) {
                return callAudio("mix", this._handle, samples);
        }
        length(stream) {
                return callAudio("length", this._handle, stream);
        }
        get sampleRate() {
                return callAudio(["getSampleRate", "sampleRate"], this._handle);
        }
        get numChannels() {
                return callAudio(["getNumChannels", "numChannels"], this._handle);
        }
        get bitsPerSample() {
                return callAudio(["getBitsPerSample", "bitsPerSample"], this._handle);
        }
        get streams() {
                return callAudio(["getStreams", "streams"], this._handle);
        }
}

class WasmAudioOut extends WasmMixer {
        constructor(dictionary) {
                super(dictionary);
                callAudio(["build", "configure"], this._handle, dictionary ?? {});
        }
        start() {
                callAudio("start", this._handle);
        }
        stop() {
                callAudio("stop", this._handle);
        }
        get mix() {}
}

export class Mixer {
        constructor(...args) {
                const Implementation = isWasmEnvironment() ? WasmMixer : XSMixer;
                return new Implementation(...args);
        }
}
Mixer.Samples = 1;
Mixer.Flush = 2;
Mixer.Callback = 3;
Mixer.Volume = 4;
Mixer.RawSamples = 5;
Mixer.Tone = 6;
Mixer.Silence = 7;

export default class AudioOut {
        constructor(...args) {
                const Implementation = isWasmEnvironment() ? WasmAudioOut : XSAudioOut;
                return new Implementation(...args);
        }
}
