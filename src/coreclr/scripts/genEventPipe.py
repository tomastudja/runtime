from __future__ import print_function
from genEventing import *
from genLttngProvider import *
import os
import xml.dom.minidom as DOM
from utilities import open_for_update, parseExclusionList, parseInclusionList

stdprolog_cpp = """// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/******************************************************************

DO NOT MODIFY. AUTOGENERATED FILE.
This file is generated using the logic from <root>/src/scripts/genEventPipe.py

******************************************************************/

"""

stdprolog_cmake = """#
#
#******************************************************************

#DO NOT MODIFY. AUTOGENERATED FILE.
#This file is generated using the logic from <root>/src/scripts/genEventPipe.py

#******************************************************************

"""

eventpipe_dirname = "eventpipe"

def generateMethodSignatureEnabled(eventName, runtimeFlavor):
    return "%s EventPipeEventEnabled%s(void)" % (getEventPipeDataTypeMapping(runtimeFlavor)["BOOL"], eventName,)

def generateMethodSignatureWrite(eventName, template, extern, runtimeFlavor):
    sig_pieces = []

    if extern: sig_pieces.append('extern "C" ')
    sig_pieces.append("%s EventPipeWriteEvent" % (getEventPipeDataTypeMapping(runtimeFlavor)["ULONG"]))
    sig_pieces.append(eventName)
    sig_pieces.append("(")

    if template:
        sig_pieces.append("\n")
        fnSig = template.signature
        for paramName in fnSig.paramlist:
            fnparam = fnSig.getParam(paramName)
            wintypeName = fnparam.winType
            typewName = getPalDataTypeMapping(runtimeFlavor)[wintypeName]
            winCount = fnparam.count
            countw = getPalDataTypeMapping(runtimeFlavor)[winCount]

            if paramName in template.structs:
                sig_pieces.append(
                    "%sint %s_ElementSize,\n" %
                    (lindent, paramName))

            sig_pieces.append(lindent)
            sig_pieces.append(typewName)
            if countw != " ":
                sig_pieces.append(countw)

            sig_pieces.append(" ")
            sig_pieces.append(fnparam.name)
            sig_pieces.append(",\n")

    sig_pieces.append(lindent)
    sig_pieces.append("%s ActivityId,\n" % (getEventPipeDataTypeMapping(runtimeFlavor)["LPCGUID"]))
    sig_pieces.append(lindent)
    sig_pieces.append("%s RelatedActivityId" % (getEventPipeDataTypeMapping(runtimeFlavor)["LPCGUID"]))
    sig_pieces.append(")")
    return ''.join(sig_pieces)

def includeProvider(providerName, runtimeFlavor):
    if runtimeFlavor.coreclr and providerName == "Microsoft-DotNETRuntimeMonoProfiler":
        return False
    else:
        return True

def includeEvent(inclusionList, providerName, eventName):
    if len(inclusionList) == 0:
        return True
    if providerName in inclusionList and eventName in inclusionList[providerName]:
        return True
    elif providerName in inclusionList and "*" in inclusionList[providerName]:
        return True
    elif "*" in inclusionList and eventName in inclusionList["*"]:
        return True
    elif "*" in inclusionList and "*" in inclusionList["*"]:
        return True
    else:
        return False

def generateClrEventPipeWriteEventsImpl(
        providerName, eventNodes, allTemplates, extern, target_cpp, runtimeFlavor, inclusionList, exclusionList):
    providerPrettyName = providerName.replace("Windows-", '')
    providerPrettyName = providerPrettyName.replace("Microsoft-", '')
    providerPrettyName = providerPrettyName.replace('-', '_')
    WriteEventImpl = []

    # EventPipeEvent declaration
    for eventNode in eventNodes:
        eventName = eventNode.getAttribute('symbol')

        if not includeEvent(inclusionList, providerName, eventName):
            continue

        WriteEventImpl.append(
            "EventPipeEvent *EventPipeEvent" +
            eventName +
            (" = nullptr;\n" if target_cpp else " = NULL;\n"))

    WriteEventImpl.append("\n")

    for eventNode in eventNodes:
        eventName = eventNode.getAttribute('symbol')
        templateName = eventNode.getAttribute('template')

        if not includeEvent(inclusionList, providerName, eventName):
            continue

        eventIsEnabledFunc = ""
        if runtimeFlavor.coreclr:
            eventIsEnabledFunc = "EventPipeAdapter::EventIsEnabled"
        elif runtimeFlavor.mono:
            eventIsEnabledFunc = "ep_event_is_enabled"

        # generate EventPipeEventEnabled function
        eventEnabledImpl = generateMethodSignatureEnabled(eventName, runtimeFlavor) + """
{
    return %s(EventPipeEvent%s);
}

""" % (eventIsEnabledFunc, eventName)
        WriteEventImpl.append(eventEnabledImpl)

        # generate EventPipeWriteEvent function
        fnptype = []

        if templateName:
            template = allTemplates[templateName]
        else:
            template = None

        fnptype.append(generateMethodSignatureWrite(eventName, template, extern, runtimeFlavor))
        fnptype.append("\n{\n")
        checking = """    if (!EventPipeEventEnabled%s())
        return ERROR_SUCCESS;
""" % (eventName)

        fnptype.append(checking)

        WriteEventImpl.extend(fnptype)

        if template:
            body = generateWriteEventBody(template, providerName, eventName, runtimeFlavor)
            WriteEventImpl.append(body)
            WriteEventImpl.append("}\n\n")
        else:
            if runtimeFlavor.coreclr:
                WriteEventImpl.append(
                    "    EventPipeAdapter::WriteEvent(EventPipeEvent" +
                    eventName +
                    ", (BYTE*) nullptr, 0, ActivityId, RelatedActivityId);\n")
            elif runtimeFlavor.mono:
                WriteEventImpl.append(
                    "    ep_write_event (EventPipeEvent" +
                    eventName +
                    ", (uint8_t *) NULL, 0, (const uint8_t *)ActivityId, (const uint8_t *)RelatedActivityId);\n")
            WriteEventImpl.append("\n    return ERROR_SUCCESS;\n}\n\n")

    # EventPipeProvider and EventPipeEvent initialization
    callbackName = 'EventPipeEtwCallback' + providerPrettyName
    createProviderFunc = ""
    if runtimeFlavor.coreclr:
        createProviderFunc = "EventPipeAdapter::CreateProvider"
    elif runtimeFlavor.mono:
        createProviderFunc = "create_provider"

    eventPipeCallbackCastExpr = ""
    if target_cpp:
        eventPipeCallbackCastExpr = "reinterpret_cast<EventPipeCallback>"
    else:
        eventPipeCallbackCastExpr = "(EventPipeCallback)"

    if runtimeFlavor.mono:
        WriteEventImpl.append("void " + callbackName + "(const uint8_t *, unsigned long, uint8_t, uint64_t,	uint64_t, EventFilterDescriptor *, void *);\n\n")

    if extern: WriteEventImpl.append('extern "C" ')
    WriteEventImpl.append(
        "void Init" +
        providerPrettyName +
        "(void);\n\n")
    if extern: WriteEventImpl.append('extern "C" ')
    WriteEventImpl.append(
        "void Init" +
        providerPrettyName +
        "(void)\n{\n")
    WriteEventImpl.append(
        "    EventPipeProvider" +
        providerPrettyName +
        " = " + createProviderFunc + "(SL(" +
        providerPrettyName +
        "Name), " + eventPipeCallbackCastExpr + "(" + callbackName + "));\n")
    for eventNode in eventNodes:
        eventName = eventNode.getAttribute('symbol')
        templateName = eventNode.getAttribute('template')
        eventKeywords = eventNode.getAttribute('keywords')
        eventKeywordsMask = generateEventKeywords(eventKeywords)
        eventValue = eventNode.getAttribute('value')
        eventVersion = eventNode.getAttribute('version')
        eventLevel = eventNode.getAttribute('level')
        eventLevel = eventLevel.replace("win:", "EP_EVENT_LEVEL_").upper()
        taskName = eventNode.getAttribute('task')

        if not includeEvent(inclusionList, providerName, eventName):
            continue

        needStack = "true"
        for nostackentry in exclusionList.nostack:
            tokens = nostackentry.split(':')
            if tokens[2] == eventName:
                needStack = "false"

        addEventFunc = ""
        if runtimeFlavor.coreclr:
            addEventFunc = "EventPipeAdapter::AddEvent"
        elif runtimeFlavor.mono:
            addEventFunc = "provider_add_event"

        initEvent = """    EventPipeEvent%s = %s(EventPipeProvider%s,%s,%s,%s,%s,%s);
""" % (eventName, addEventFunc, providerPrettyName, eventValue, eventKeywordsMask, eventVersion, eventLevel, needStack)

        WriteEventImpl.append(initEvent)
    WriteEventImpl.append("}")

    return ''.join(WriteEventImpl)


def generateWriteEventBody(template, providerName, eventName, runtimeFlavor):
    fnSig = template.signature
    pack_list = []

    # Write out replacement for any instances of NULL for UnicodeString type input.
    # ETW translates these to "NULL", so we are doing the same for EventPipe.
    for paramName in fnSig.paramlist:
        parameter = fnSig.getParam(paramName)

        if parameter.winType == "win:UnicodeString":
            if runtimeFlavor.coreclr:
                pack_list.append(
                    "    if (!%s) { %s = W(\"NULL\"); }" %
                    (parameter.name, parameter.name))
            elif runtimeFlavor.mono:
                pack_list.append(
                    "    if (!%s) { %s = (const ep_char8_t *)\"NULL\"; }" %
                    (parameter.name, parameter.name))

    emittedWriteToBuffer = False
    for paramName in fnSig.paramlist:
        parameter = fnSig.getParam(paramName)

        if paramName in template.structs:
            size = "(int)%s_ElementSize * (int)%s" % (
                paramName, parameter.prop)
            if template.name in specialCaseSizes and paramName in specialCaseSizes[template.name]:
                size = "(int)(%s)" % specialCaseSizes[template.name][paramName]
            if runtimeFlavor.mono:
                pack_list.append(
                    "    success &= write_buffer((const uint8_t *)%s, %s, &buffer, &offset, &size, &fixedBuffer);" %
                    (paramName, size))
                emittedWriteToBuffer = True
            elif runtimeFlavor.coreclr:
                pack_list.append(
                    "    success &= WriteToBuffer((const BYTE *)%s, %s, buffer, offset, size, fixedBuffer);" %
                    (paramName, size))
                emittedWriteToBuffer = True
        elif paramName in template.arrays:
            size = "sizeof(%s) * (int)%s" % (
                lttngDataTypeMapping[parameter.winType],
                parameter.prop)
            if template.name in specialCaseSizes and paramName in specialCaseSizes[template.name]:
                size = "(int)(%s)" % specialCaseSizes[template.name][paramName]
            if runtimeFlavor.mono:
                pack_list.append(
                    "    success &= write_buffer((const uint8_t *)%s, %s, &buffer, &offset, &size, &fixedBuffer);" %
                    (paramName, size))
                emittedWriteToBuffer = True
            elif runtimeFlavor.coreclr:
                pack_list.append(
                    "    success &= WriteToBuffer((const BYTE *)%s, %s, buffer, offset, size, fixedBuffer);" %
                    (paramName, size))
                emittedWriteToBuffer = True
        elif parameter.winType == "win:GUID" and runtimeFlavor.mono:
            pack_list.append(
                "    success &= write_buffer_guid_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:GUID":
            pack_list.append(
                "    success &= WriteToBuffer(*%s, buffer, offset, size, fixedBuffer);" %
                (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:AnsiString" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_string_utf8_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:UnicodeString" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_string_utf8_to_utf16_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:UInt8" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_uint8_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:UInt16" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_uint16_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:Int32" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_int32_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:UInt32" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_uint32_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:Int64" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_int64_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:UInt64" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_uint64_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:Boolean" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_bool_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:Double" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_double_t(%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif parameter.winType == "win:Pointer" and runtimeFlavor.mono:
            pack_list.append(
                    "    success &= write_buffer_uintptr_t((uintptr_t)%s, &buffer, &offset, &size, &fixedBuffer);" %
                    (parameter.name,))
            emittedWriteToBuffer = True
        elif runtimeFlavor.mono:
            pack_list.append(
                "    success &= write_buffer((const uint8_t *)%s, sizeof(%s), &buffer, &offset, &size, &fixedBuffer);" %
                (parameter.name,parameter.name,))
            emittedWriteToBuffer = True
        elif runtimeFlavor.coreclr:
            pack_list.append(
                "    success &= WriteToBuffer(%s, buffer, offset, size, fixedBuffer);" %
                (parameter.name,))
            emittedWriteToBuffer = True

    code = "\n".join(pack_list) + "\n\n"

    header = """
    size_t size = {0:d};
    {1:s} stackBuffer[{0:d}];
    {1:s} *buffer = stackBuffer;
    size_t offset = 0;
""".format(template.estimated_size, getEventPipeDataTypeMapping(runtimeFlavor)["BYTE"])

    checking = ""
    if emittedWriteToBuffer:
        header += """    bool fixedBuffer = true;
    bool success = true;
"""
        if runtimeFlavor.coreclr:
            checking = """    if (!success)
    {
        if (!fixedBuffer)
            delete[] buffer;
        return ERROR_WRITE_FAULT;
    }\n\n"""
        elif runtimeFlavor.mono:
            checking = """    ep_raise_error_if_nok (success);\n\n"""

    body = ""
    if runtimeFlavor.coreclr:
        body = "    EventPipeAdapter::WriteEvent(EventPipeEvent" + \
            eventName + ", (BYTE *)buffer, (unsigned int)offset, ActivityId, RelatedActivityId);\n"
    elif runtimeFlavor.mono:
        body = "    ep_write_event (EventPipeEvent" + \
            eventName + ", (uint8_t *)buffer, (uint32_t)offset, ActivityId, RelatedActivityId);\n"

    header += "\n"
    footer = ""
    if emittedWriteToBuffer:
        if runtimeFlavor.coreclr:
            footer = """
    if (!fixedBuffer)
        delete[] buffer;

"""

    if runtimeFlavor.coreclr:
        footer += """
    return ERROR_SUCCESS;
"""
    elif runtimeFlavor.mono:
        footer += """
ep_on_exit:
    if (!fixedBuffer)
        ep_rt_byte_array_free (buffer);
    return success ? ERROR_SUCCESS : ERROR_WRITE_FAULT;

ep_on_error:
    EP_ASSERT (!success);
    ep_exit_error_handler ();
"""

    return header + code + checking + body + footer


keywordMap = {}

def generateEventKeywords(eventKeywords):
    mask = 0
    # split keywords if there are multiple
    allKeywords = eventKeywords.split()

    for singleKeyword in allKeywords:
        mask = mask | keywordMap[singleKeyword]

    return mask

def getCoreCLREventPipeHelperFileImplPrefix():
    return """
#include "common.h"
#include <stdlib.h>
#include <string.h>

#ifndef TARGET_UNIX
#include <windef.h>
#include <crtdbg.h>
#else
#include "pal.h"
#endif //TARGET_UNIX

bool ResizeBuffer(BYTE *&buffer, size_t& size, size_t currLen, size_t newSize, bool &fixedBuffer)
{
    newSize = (size_t)(newSize * 1.5);
    _ASSERTE(newSize > size); // check for overflow

    if (newSize < 32)
        newSize = 32;

    BYTE *newBuffer = new (nothrow) BYTE[newSize];

    if (newBuffer == NULL)
        return false;

    memcpy(newBuffer, buffer, currLen);

    if (!fixedBuffer)
        delete[] buffer;

    buffer = newBuffer;
    size = newSize;
    fixedBuffer = false;

    return true;
}

bool WriteToBuffer(const BYTE *src, size_t len, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer)
{
    if (!src) return true;
    if (offset + len > size)
    {
        if (!ResizeBuffer(buffer, size, offset, size + len, fixedBuffer))
            return false;
    }

    memcpy(buffer + offset, src, len);
    offset += len;
    return true;
}

bool WriteToBuffer(PCWSTR str, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer)
{
    if (!str) return true;
    size_t byteCount = (wcslen(str) + 1) * sizeof(*str);

    if (offset + byteCount > size)
    {
        if (!ResizeBuffer(buffer, size, offset, size + byteCount, fixedBuffer))
            return false;
    }

    memcpy(buffer + offset, str, byteCount);
    offset += byteCount;
    return true;
}

bool WriteToBuffer(const char *str, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer)
{
    if (!str) return true;
    size_t len = strlen(str) + 1;
    if (offset + len > size)
    {
        if (!ResizeBuffer(buffer, size, offset, size + len, fixedBuffer))
            return false;
    }

    memcpy(buffer + offset, str, len);
    offset += len;
    return true;
}

"""

def getCoreCLREventPipeHelperFileImplSuffix():
    return ""

def getMonoEventPipeHelperFileImplPrefix():
    return """#include <config.h>
#ifdef ENABLE_PERFTRACING
#include <eventpipe/ep-rt-config.h>
#include <eventpipe/ep-types.h>
#include <eventpipe/ep-rt.h>
#include <eventpipe/ep.h>
#include <eventpipe/ep-event.h>

bool
resize_buffer (
    uint8_t **buffer,
    size_t *size,
    size_t current_size,
    size_t new_size,
    bool *fixed_buffer);

bool
write_buffer (
    const uint8_t *value,
    size_t value_size,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer);

bool
write_buffer_string_utf8_to_utf16_t (
    const ep_char8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer);

bool
write_buffer_string_utf8_t (
    const ep_char8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer);

bool
resize_buffer (
    uint8_t **buffer,
    size_t *size,
    size_t current_size,
    size_t new_size,
    bool *fixed_buffer)
{
    EP_ASSERT (buffer != NULL);
    EP_ASSERT (size != NULL);
    EP_ASSERT (fixed_buffer != NULL);

    new_size = (size_t)(new_size * 1.5);
    if (new_size < *size) {
        EP_ASSERT (!"Overflow");
        return false;
    }

    if (new_size < 32)
        new_size = 32;

    uint8_t *new_buffer;
    new_buffer = ep_rt_byte_array_alloc (new_size);
    ep_raise_error_if_nok (new_buffer != NULL);

    memcpy (new_buffer, *buffer, current_size);

    if (!*fixed_buffer)
        ep_rt_byte_array_free (*buffer);

    *buffer = new_buffer;
    *size = new_size;
    *fixed_buffer = false;

    return true;

    ep_on_error:
    return false;
}

bool
write_buffer (
    const uint8_t *value,
    size_t value_size,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    EP_ASSERT (buffer != NULL);
    EP_ASSERT (offset != NULL);
    EP_ASSERT (size != NULL);
    EP_ASSERT (fixed_buffer != NULL);

    if ((value_size + *offset) > *size)
        ep_raise_error_if_nok (resize_buffer (buffer, size, *offset, *size + value_size, fixed_buffer));

    memcpy (*buffer + *offset, value, value_size);
    *offset += value_size;

    return true;

ep_on_error:
    return false;
}

bool
write_buffer_string_utf8_to_utf16_t (
    const ep_char8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    if (!value)
        return true;

    GFixedBufferCustomAllocatorData custom_alloc_data;
    custom_alloc_data.buffer = *buffer + *offset;
    custom_alloc_data.buffer_size = *size - *offset;
    custom_alloc_data.req_buffer_size = 0;

    if (!g_utf8_to_utf16_custom_alloc (value, -1, NULL, NULL, g_fixed_buffer_custom_allocator, &custom_alloc_data, NULL)) {
        ep_raise_error_if_nok (resize_buffer (buffer, size, *offset, *size + custom_alloc_data.req_buffer_size, fixed_buffer));
        custom_alloc_data.buffer = *buffer + *offset;
        custom_alloc_data.buffer_size = *size - *offset;
        custom_alloc_data.req_buffer_size = 0;
        ep_raise_error_if_nok (g_utf8_to_utf16_custom_alloc (value, -1, NULL, NULL, g_fixed_buffer_custom_allocator, &custom_alloc_data, NULL) != NULL);
    }

    *offset += custom_alloc_data.req_buffer_size;
    return true;

ep_on_error:
    return false;
}

bool
write_buffer_string_utf8_t (
    const ep_char8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    if (!value)
        return true;

    size_t value_len = 0;
    while (value [value_len])
        value_len++;

    return write_buffer ((const uint8_t *)value, (value_len + 1) * sizeof(*value), buffer, offset, size, fixed_buffer);
}

"""

def getMonoEventPipeHelperFileImplSuffix():
    return "#endif\n"

def generateEventPipeHelperFile(etwmanifest, eventpipe_directory, target_cpp, runtimeFlavor, extern, dryRun):
    eventpipehelpersPath = os.path.join(eventpipe_directory, "eventpipehelpers" + (".cpp" if target_cpp else ".c"))
    if dryRun:
        print(eventpipehelpersPath)
    else:
        with open_for_update(eventpipehelpersPath) as helper:
            helper.write(stdprolog_cpp)
            if runtimeFlavor.coreclr:
                helper.write(getCoreCLREventPipeHelperFileImplPrefix())
            elif runtimeFlavor.mono:
                helper.write(getMonoEventPipeHelperFileImplPrefix())

            tree = DOM.parse(etwmanifest)

            for providerNode in tree.getElementsByTagName('provider'):
                providerName = providerNode.getAttribute('name')
                if includeProvider(providerName, runtimeFlavor):
                    providerPrettyName = providerName.replace("Windows-", '')
                    providerPrettyName = providerPrettyName.replace("Microsoft-", '')
                    providerPrettyName = providerPrettyName.replace('-', '_')
                    if extern: helper.write(
                        'extern "C" '
                    )
                    helper.write(
                        "void Init" +
                        providerPrettyName +
                        "(void);\n\n")

            if extern: helper.write(
                'extern "C" '
            )
            helper.write("void InitProvidersAndEvents(void);\n\n")
            helper.write("void InitProvidersAndEvents(void)\n{\n")
            for providerNode in tree.getElementsByTagName('provider'):
                providerName = providerNode.getAttribute('name')
                if includeProvider(providerName, runtimeFlavor):
                    providerPrettyName = providerName.replace("Windows-", '')
                    providerPrettyName = providerPrettyName.replace("Microsoft-", '')
                    providerPrettyName = providerPrettyName.replace('-', '_')
                    helper.write("    Init" + providerPrettyName + "();\n")
            helper.write("}\n")

            if runtimeFlavor.coreclr:
                helper.write(getCoreCLREventPipeHelperFileImplSuffix())
            elif runtimeFlavor.mono:
                helper.write(getMonoEventPipeHelperFileImplSuffix())

        helper.close()

def getCoreCLREventPipeImplFilePrefix():
    return """#include <common.h>
#include "eventpipeadapter.h"

#if defined(TARGET_UNIX)
#define wcslen PAL_wcslen
#endif

bool ResizeBuffer(BYTE *&buffer, size_t& size, size_t currLen, size_t newSize, bool &fixedBuffer);
bool WriteToBuffer(PCWSTR str, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer);
bool WriteToBuffer(const char *str, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer);
bool WriteToBuffer(const BYTE *src, size_t len, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer);

template <typename T>
bool WriteToBuffer(const T &value, BYTE *&buffer, size_t& offset, size_t& size, bool &fixedBuffer)
{
    if (sizeof(T) + offset > size)
    {
        if (!ResizeBuffer(buffer, size, offset, size + sizeof(T), fixedBuffer))
            return false;
    }

    memcpy(buffer + offset, (char *)&value, sizeof(T));
    offset += sizeof(T);
    return true;
}
"""

def getCoreCLREventPipeImplFileSuffix():
    return ""

def getMonoEventPipeImplFilePrefix():
    return """#include <eventpipe/ep-rt-config.h>
#ifdef ENABLE_PERFTRACING
#include <eventpipe/ep.h>
#include <eventpipe/ep-event.h>
#include "clreventpipewriteevents.h"
%s
/*
 * Forward declares of functions.
 */

bool
resize_buffer (
    uint8_t **buffer,
    size_t *size,
    size_t current_size,
    size_t new_size,
    bool *fixed_buffer);

bool
write_buffer (
    const uint8_t *value,
    size_t value_size,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer);

bool
write_buffer_string_utf8_t (
    const ep_char8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer);

bool
write_buffer_string_utf8_to_utf16_t (
    const ep_char8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer);

static
inline
bool
write_buffer_guid_t (
    const uint8_t *value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer (value, EP_GUID_SIZE, buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_uint8_t (
    uint8_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (uint8_t), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_uint16_t (
    uint16_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (uint16_t), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_uint32_t (
    uint32_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (uint32_t), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_int32_t (
    int32_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (int32_t), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_uint64_t (
    uint64_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (uint64_t), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_int64_t (
    int64_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (int64_t), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_double_t (
    double value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (double), buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_bool_t (
    bool value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer_int32_t (value, buffer, offset, size, fixed_buffer);
}

static
inline
bool
write_buffer_uintptr_t (
    uintptr_t value,
    uint8_t **buffer,
    size_t *offset,
    size_t *size,
    bool *fixed_buffer)
{
    return write_buffer ((const uint8_t *)&value, sizeof (uintptr_t), buffer, offset, size, fixed_buffer);
}

static
inline
EventPipeEvent *
provider_add_event (
    EventPipeProvider *provider,
    uint32_t event_id,
    uint64_t keywords,
    uint32_t event_version,
    EventPipeEventLevel level,
    bool need_stack)
{
    return ep_provider_add_event (provider, event_id, keywords, event_version, level, need_stack, NULL, 0);
}

static
inline
EventPipeProvider *
create_provider (
    const wchar_t *provider_name,
    EventPipeCallback callback_func)
{
    ep_char8_t *provider_name_utf8 = NULL;

#if WCHAR_MAX == 0xFFFF
    provider_name_utf8 = g_utf16_to_utf8 ((const gunichar2 *)provider_name, -1, NULL, NULL, NULL);
#else
    provider_name_utf8 = g_ucs4_to_utf8 ((const gunichar *)provider_name, -1, NULL, NULL, NULL);
#endif

    ep_return_null_if_nok (provider_name_utf8 != NULL);

    EventPipeProvider *provider = ep_create_provider (provider_name_utf8, callback_func, NULL, NULL);

    g_free (provider_name_utf8);
    return provider;
}
""" % (getCoreCLRMonoTypeAdaptionDefines())

def getMonoEventPipeImplFileSuffix():
    return "#endif\n"

def generateEventPipeImplFiles(
        etwmanifest, eventpipe_directory, extern, target_cpp, runtimeFlavor, inclusionList, exclusionList, dryRun):
    tree = DOM.parse(etwmanifest)

    # Find the src directory starting with the assumption that
    # A) It is named 'src'
    # B) This script lives in it
    src_dirname = os.path.dirname(__file__)
    while os.path.basename(src_dirname) != "src":
        src_dirname = os.path.dirname(src_dirname)

        if os.path.basename(src_dirname) == "":
            raise IOError("Could not find the Core CLR 'src' directory")

    for providerNode in tree.getElementsByTagName('provider'):
        providerName = providerNode.getAttribute('name')
        if not includeProvider(providerName, runtimeFlavor):
            continue

        providerPrettyName = providerName.replace("Windows-", '')
        providerPrettyName = providerPrettyName.replace("Microsoft-", '')

        providerName_File = providerPrettyName.replace('-', '')
        if target_cpp:
            providerName_File = providerName_File + ".cpp"
        else:
            providerName_File = providerName_File + ".c"

        providerName_File = providerName_File.lower()

        eventpipefile = os.path.join(eventpipe_directory, providerName_File)

        providerPrettyName = providerPrettyName.replace('-', '_')

        if dryRun:
            print(eventpipefile)
        else:
            with open_for_update(eventpipefile) as eventpipeImpl:
                eventpipeImpl.write(stdprolog_cpp)
                header = ""
                if runtimeFlavor.coreclr:
                    header = getCoreCLREventPipeImplFilePrefix()
                elif runtimeFlavor.mono:
                    header = getMonoEventPipeImplFilePrefix()

                eventpipeImpl.write(header + "\n")
                eventpipeImpl.write(
                    "const %s* %sName = W(\"%s\");\n" % (
                        getEventPipeDataTypeMapping(runtimeFlavor)["WCHAR"],
                        providerPrettyName,
                        providerName
                    )
                )

                eventpipeImpl.write(
                    "EventPipeProvider *EventPipeProvider" + providerPrettyName + 
                    (" = nullptr;\n" if target_cpp else " = NULL;\n")
                )
                templateNodes = providerNode.getElementsByTagName('template')
                allTemplates = parseTemplateNodes(templateNodes)
                eventNodes = providerNode.getElementsByTagName('event')
                eventpipeImpl.write(
                    generateClrEventPipeWriteEventsImpl(
                        providerName,
                        eventNodes,
                        allTemplates,
                        extern,
                        target_cpp,
                        runtimeFlavor,
                        inclusionList,
                        exclusionList) + "\n")

                if runtimeFlavor.coreclr:
                    eventpipeImpl.write(getCoreCLREventPipeImplFileSuffix())
                elif runtimeFlavor.mono:
                    eventpipeImpl.write(getMonoEventPipeImplFileSuffix())

def generateEventPipeFiles(
        etwmanifest, intermediate, extern, target_cpp, runtimeFlavor, inclusionList, exclusionList, dryRun):
    eventpipe_directory = os.path.join(intermediate, eventpipe_dirname)
    tree = DOM.parse(etwmanifest)

    if not os.path.exists(eventpipe_directory):
        os.makedirs(eventpipe_directory)

    # generate helper file
    generateEventPipeHelperFile(etwmanifest, eventpipe_directory, target_cpp, runtimeFlavor, extern, dryRun)

    # generate all keywords
    for keywordNode in tree.getElementsByTagName('keyword'):
        keywordName = keywordNode.getAttribute('name')
        keywordMask = keywordNode.getAttribute('mask')
        keywordMap[keywordName] = int(keywordMask, 0)

    # generate file for each provider
    generateEventPipeImplFiles(
        etwmanifest,
        eventpipe_directory,
        extern,
        target_cpp,
        runtimeFlavor,
        inclusionList,
        exclusionList,
        dryRun
    )

import argparse
import sys

def main(argv):

    # parse the command line
    parser = argparse.ArgumentParser(
        description="Generates the Code required to instrument eventpipe logging mechanism")

    required = parser.add_argument_group('required arguments')
    required.add_argument('--man', type=str, required=True,
                          help='full path to manifest containig the description of events')
    required.add_argument('--exc',  type=str, required=True,
                                    help='full path to exclusion list')
    required.add_argument('--inc',  type=str,default="",
                                    help='full path to inclusion list')
    required.add_argument('--intermediate', type=str, required=True,
                          help='full path to eventprovider  intermediate directory')
    required.add_argument('--runtimeflavor', type=str,default="CoreCLR",
                          help='runtime flavor')
    required.add_argument('--nonextern', action='store_true',
                          help='if specified, will generate files to be compiled into the CLR rather than extern' )
    required.add_argument('--dry-run', action='store_true',
                                    help='if specified, will output the names of the generated files instead of generating the files' )
    args, unknown = parser.parse_known_args(argv)
    if unknown:
        print('Unknown argument(s): ', ', '.join(unknown))
        return 1

    sClrEtwAllMan = args.man
    exclusion_filename = args.exc
    inclusion_filename = args.inc
    intermediate = args.intermediate
    runtimeFlavor = RuntimeFlavor(args.runtimeflavor)
    extern = not args.nonextern
    dryRun = args.dry_run

    target_cpp = True
    if runtimeFlavor.mono:
        extern = False
        target_cpp = False

    inclusion_list = parseInclusionList(inclusion_filename)
    exclusion_list = parseExclusionList(exclusion_filename)

    generateEventPipeFiles(sClrEtwAllMan, intermediate, extern, target_cpp, runtimeFlavor, inclusion_list, exclusion_list, dryRun)

if __name__ == '__main__':
    return_code = main(sys.argv[1:])
    sys.exit(return_code)
