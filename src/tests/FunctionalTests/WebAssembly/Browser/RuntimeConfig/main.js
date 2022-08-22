import { dotnet } from './dotnet.js'

function wasm_exit(exit_code) {
    var tests_done_elem = document.createElement("label");
    tests_done_elem.id = "tests_done";
    tests_done_elem.innerHTML = exit_code.toString();
    document.body.appendChild(tests_done_elem);

    console.log(`WASM EXIT ${exit_code}`);
}

try {
    const { getAssemblyExports } = await dotnet.create();
    const exports = await getAssemblyExports("WebAssembly.Browser.RuntimeConfig.Test.dll");
    const testMeaning = exports.Sample.Test.TestMeaning;
    const ret = testMeaning();
    document.getElementById("out").innerHTML = `${ret}`;
    console.debug(`ret: ${ret}`);

    let exit_code = ret;
    wasm_exit(exit_code);
} catch (err) {
    console.log(`WASM ERROR ${err}`);
}
