const GLOBAL_KEY = "__moddableWasmHost";

let wasmHost = globalThis[GLOBAL_KEY] ?? null;

export function registerWasmHost(host) {
        if ((null !== host) && ("object" !== typeof host))
                throw new TypeError("WASM host must be an object or null");

        wasmHost = host ?? null;
        if (null === wasmHost)
                delete globalThis[GLOBAL_KEY];
        else
                globalThis[GLOBAL_KEY] = wasmHost;
}

export function getWasmHost() {
        return wasmHost;
}

export function isWasmEnvironment() {
        return !!wasmHost;
}

export function assertWasmHost() {
        if (!wasmHost)
                throw new Error("WASM host not registered");
        return wasmHost;
}

export function requireCapability(name) {
        const host = assertWasmHost();
        let capability;

        if ("function" === typeof host.getCapability)
                capability = host.getCapability(name);
        else
                capability = host[name];

        if (undefined === capability)
                throw new Error(`WASM host missing capability \"${name}\"`);
        return capability;
}

export function callCapabilityMethod(name, method, ...args) {
        const capability = requireCapability(name);
        const fn = capability?.[method];
        if ("function" !== typeof fn)
                throw new Error(`WASM host capability \"${name}\" is missing method \"${method}\"`);
        return fn.apply(capability, args);
}

export function callCapabilityFunction(name, method, ...args) {
        const capability = requireCapability(name);
        const fn = capability?.[method];
        if ("function" !== typeof fn)
                throw new Error(`WASM host capability \"${name}\" is missing method \"${method}\"`);
        return fn(...args);
}
