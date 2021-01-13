// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
var Module = {
    onRuntimeInitialized: function () {
        config.loaded_cb = function () {
            try {
                App.init ();
            } catch (error) {
                test_exit(1);
                throw (error);
            }
        };
        config.fetch_file_cb = function (asset) {
            return fetch (asset, { credentials: 'same-origin' });
        }

        if (config.enable_profiler)
        {
            config.aot_profiler_options = {
                write_at:"Sample.Test::StopProfile",
                send_to: "System.Runtime.InteropServices.JavaScript.Runtime::DumpAotProfileData"
            }
        }

        try
        {
            MONO.mono_load_runtime_and_bcl_args (config);
        } catch (error) {
            test_exit(1);
            throw(error);
        }
    }
};