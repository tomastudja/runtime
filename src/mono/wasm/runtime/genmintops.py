#!/usr/bin/env python3

#
# This tool is used to generate the typescript version of mintops.def, used by the WASM jiterpreter.
#

import sys
import os
import re

if len (sys.argv) != 3:
    print ("Usage: genmintops.py <src/mintops.def> <dest/mintops.ts>")
    exit (1)

src_header_path = sys.argv [1]
output_ts_path = sys.argv [2]

src = open(src_header_path, 'r')

tab = "    "
header = tab + src.read().replace("\n", "\n" + tab)
src.close()

opdef_regex = r'OPDEF\((\w+),\s*(.+?),\s*(MintOp\w+)\)'
enum_values = re.sub(
    opdef_regex, lambda m : f"{m.group(1)}{' = 0' if (m.group(1) == 'MINT_NOP') else ''},", header
)
metadata_table = re.sub(
    opdef_regex, lambda m : f"[MintOpcode.{m.group(1)}]: [{m.group(2)}, MintOpArgType.{m.group(3)}],", header
)

generated = f"""
// Generated by genmintops.py from mintops.def.
// Do not manually edit this file.

import {{ OpcodeInfoTable, MintOpArgType }} from "./jiterpreter-opcodes";

export const enum MintOpcode {{
{enum_values}

    MINT_LASTOP
}}

export const OpcodeInfo : OpcodeInfoTable = {{
{metadata_table}
}};
"""

os.makedirs(os.path.dirname(output_ts_path), exist_ok=True)
try:
    with open(output_ts_path, 'r') as dest:
        if (dest.read() == generated):
            print("mintops.ts up to date, exiting")
            exit(0)
except FileNotFoundError:
    pass
with open(output_ts_path, 'w') as dest:
    dest.write(generated)
print("mintops.ts generated")
exit(0)
