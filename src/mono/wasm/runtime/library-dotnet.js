/* eslint-disable no-undef */
// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

"use strict";

const DotNetSupportLib = {
    // this will become globalThis.DOTNET
    $DOTNET: {},
    // this will become globalThis.MONO
    $MONO: {},
    // this will become globalThis.BINDING
    $BINDING: {},
    // this will become globalThis.INTERNAL
    $INTERNAL: {},
    // this line will be executed on runtime, populating the objects with methods
    $DOTNET__postset: "__dotnet_runtime.INTERNAL.export_to_emscripten (DOTNET, MONO, BINDING, INTERNAL, Module);",
};

// the methods would be visible to EMCC linker
// --- keep in sync with exports.ts ---
const linked_functions = [
    // mini-wasm.c
    "mono_set_timeout",

    // mini-wasm-debugger.c
    "mono_wasm_asm_loaded",
    "mono_wasm_fire_debugger_agent_message",

    // mono-threads-wasm.c
    "schedule_background_exec",

    // driver.c
    "mono_wasm_invoke_js_blazor",
    "mono_wasm_invoke_js_marshalled",
    "mono_wasm_invoke_js_unmarshalled",

    // corebindings.c
    "mono_wasm_invoke_js_with_args",
    "mono_wasm_get_object_property",
    "mono_wasm_set_object_property",
    "mono_wasm_get_by_index",
    "mono_wasm_set_by_index",
    "mono_wasm_get_global_object",
    "mono_wasm_create_cs_owned_object",
    "mono_wasm_release_cs_owned_object",
    "mono_wasm_typed_array_to_array",
    "mono_wasm_typed_array_copy_to",
    "mono_wasm_typed_array_from",
    "mono_wasm_typed_array_copy_from",
    "mono_wasm_add_event_listener",
    "mono_wasm_remove_event_listener",
    "mono_wasm_cancel_promise",
    "mono_wasm_web_socket_open",
    "mono_wasm_web_socket_send",
    "mono_wasm_web_socket_receive",
    "mono_wasm_web_socket_close",
    "mono_wasm_web_socket_abort",

    // pal_icushim_static.c
    "mono_wasm_load_icu_data",
    "mono_wasm_get_icudt_name",
];

// -- this javascript file is evaluated by emcc during compilation! --
// we generate simple proxy for each exported function so that emcc will include them in the final output
for (let linked_function of linked_functions) {
    const fn_template = `return __dotnet_runtime.INTERNAL.linker_exports.${linked_function}.apply(__dotnet_runtime, arguments)`;
    DotNetSupportLib[linked_function] = new Function(fn_template);
}

autoAddDeps(DotNetSupportLib, "$DOTNET");
autoAddDeps(DotNetSupportLib, "$MONO");
autoAddDeps(DotNetSupportLib, "$BINDING");
autoAddDeps(DotNetSupportLib, "$INTERNAL");
mergeInto(LibraryManager.library, DotNetSupportLib);
