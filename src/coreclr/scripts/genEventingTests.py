# Python 2 compatibility
from __future__ import print_function

import os
import xml.dom.minidom as DOM
from genEventing import parseTemplateNodes
from utilities import open_for_update

stdprolog="""
// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/******************************************************************

DO NOT MODIFY. AUTOGENERATED FILE.
This file is generated using the logic from <root>/src/scripts/genEventing.py

******************************************************************/
"""


def generateClralltestEvents(sClrEtwAllMan):
    tree           = DOM.parse(sClrEtwAllMan)

    clrtestEvents = []
    for providerNode in tree.getElementsByTagName('provider'):
        templateNodes = providerNode.getElementsByTagName('template')
        allTemplates  = parseTemplateNodes(templateNodes)
        eventNodes = providerNode.getElementsByTagName('event')
        for eventNode in eventNodes:
            eventName    = eventNode.getAttribute('symbol')
            templateName = eventNode.getAttribute('template')
            clrtestEvents.append(" EventXplatEnabled" + eventName + "();\n")
            clrtestEvents.append("Error |= FireEtXplat" + eventName + "(\n")

            line =[]
            if templateName:
                template = allTemplates[templateName]
                fnSig = template.signature

                for params in fnSig.paramlist:
                    if params in template.structs:
                        line.append("sizeof(Struct1),\n")

                    argline =''
                    fnparam     = fnSig.getParam(params)
                    if fnparam.name.lower() == 'count':
                        argline = '2'
                    else:
                        if fnparam.winType == "win:Binary":
                            argline = 'win_Binary'
                        elif fnparam.winType == "win:Pointer" and fnparam.count == "win:count":
                            argline = "(const void**)&var11"
                        elif fnparam.winType == "win:Pointer" :
                            argline = "(const void*)var11"
                        elif fnparam.winType =="win:AnsiString":
                            argline    = '" Testing AniString "'
                        elif fnparam.winType =="win:UnicodeString":
                            argline    = 'W(" Testing UnicodeString ")'
                        else:
                            if fnparam.count == "win:count":
                                line.append("&")

                            argline = fnparam.winType.replace(":","_")

                    line.append(argline)
                    line.append(",\n")

                #remove trailing commas
                if len(line) > 0:
                    del line[-1]
                    line.append("\n")
            line.append(");\n")
            clrtestEvents.extend(line)

    return ''.join(clrtestEvents)

def generateSanityTest(sClrEtwAllMan,testDir):

    if not os.path.exists(testDir):
        os.makedirs(testDir)

    test_cpp   = testDir + "/clralltestevents.cpp"
    testinfo   = testDir + "/testinfo.dat"

    with open_for_update(testinfo) as Testinfo:
        Testinfo.write("""
Copyright (c) Microsoft Corporation.  All rights reserved.
#

Version = 1.0
Section = EventProvider
Function = EventProvider
Name = PAL test for FireEtW* and EventEnabled* functions
TYPE = DEFAULT
EXE1 = eventprovidertest
Description = This is a sanity test to check that there are no crashes in Xplat eventing
""")

    #Test.cpp
    with open_for_update(test_cpp) as Test_cpp:
        Test_cpp.write(stdprolog)
        Test_cpp.write("""
/*=====================================================================
**
** Source:   clralltestevents.cpp
**
** Purpose:  Ensure Correctness of Eventing code
**
**
**===================================================================*/
#if TARGET_UNIX
#include <palsuite.h>
#endif //TARGET_UNIX
#include <clrxplatevents.h>

typedef struct _Struct1 {
                ULONG   Data1;
                unsigned short Data2;
                unsigned short Data3;
                unsigned char  Data4[8];
} Struct1;

Struct1 var21[2] = { { 245, 13, 14, "deadbea" }, { 542, 0, 14, "deadflu" } };

Struct1* var11 = var21;
Struct1* win_Struct = var21;

GUID win_GUID ={ 245, 13, 14, "deadbea" };
double win_Double =34.04;
ULONG win_ULong = 34;
BOOL win_Boolean = FALSE;
unsigned __int64 win_UInt64 = 114;
__int64 win_Int64 = 114;
unsigned int win_UInt32 = 4;
unsigned short win_UInt16 = 12;
unsigned char win_UInt8 = 9;
int win_Int32 = 12;
BYTE* win_Binary =(BYTE*)var21 ;
int __cdecl main(int argc, char **argv)
{
#if defined(TARGET_UNIX)
            /* Initialize the PAL.
            */

            if(0 != PAL_Initialize(argc, argv))
            {
            return FAIL;
            }
#endif

            ULONG Error = ERROR_SUCCESS;
#if defined(FEATURE_EVENT_TRACE)
            Trace("\\n Starting functional  eventing APIs tests  \\n");
""")

        Test_cpp.write(generateClralltestEvents(sClrEtwAllMan))
        Test_cpp.write("""

        if (Error != ERROR_SUCCESS)
        {
            Fail("One or more eventing Apis failed\\n ");
            return FAIL;
        }
        Trace("\\n All eventing APIs were fired succesfully \\n");
#endif //defined(FEATURE_EVENT_TRACE)
#if defined(TARGET_UNIX)

/* Shutdown the PAL.
*/

        PAL_Terminate();
#endif
        return PASS;
                                }

""")
    Testinfo.close()


import argparse
import sys

def main(argv):

    #parse the command line
    parser = argparse.ArgumentParser(description="Generates the Code required to instrument LTTtng logging mechanism")

    required = parser.add_argument_group('required arguments')
    required.add_argument('--man',  type=str, required=True,
                                    help='full path to manifest containig the description of events')
    required.add_argument('--testdir', type=str, required=True,
                                    help='full path to directory where the test assets will be deployed' )
    args, unknown = parser.parse_known_args(argv)
    if unknown:
        print('Unknown argument(s): ', ', '.join(unknown))
        return 1

    sClrEtwAllMan     = args.man
    testDir           = args.testdir

    generateSanityTest(sClrEtwAllMan, testDir)

if __name__ == '__main__':
    return_code = main(sys.argv[1:])
    sys.exit(return_code)
