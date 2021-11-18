// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json.Linq;
using System.IO;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using System.Collections.Generic;

namespace Microsoft.WebAssembly.Diagnostics
{
    internal class MemberReferenceResolver
    {
        private SessionId sessionId;
        private int scopeId;
        private MonoProxy proxy;
        private ExecutionContext context;
        private PerScopeCache scopeCache;
        private ILogger logger;
        private bool localsFetched;
        private int linqTypeId;

        public MemberReferenceResolver(MonoProxy proxy, ExecutionContext ctx, SessionId sessionId, int scopeId, ILogger logger)
        {
            this.sessionId = sessionId;
            this.scopeId = scopeId;
            this.proxy = proxy;
            this.context = ctx;
            this.logger = logger;
            scopeCache = ctx.GetCacheForScope(scopeId);
            linqTypeId = -1;
        }

        public MemberReferenceResolver(MonoProxy proxy, ExecutionContext ctx, SessionId sessionId, JArray objectValues, ILogger logger)
        {
            this.sessionId = sessionId;
            scopeId = -1;
            this.proxy = proxy;
            this.context = ctx;
            this.logger = logger;
            scopeCache = new PerScopeCache(objectValues);
            localsFetched = true;
            linqTypeId = -1;
        }

        public async Task<JObject> GetValueFromObject(JToken objRet, CancellationToken token)
        {
            if (objRet["value"]?["className"]?.Value<string>() == "System.Exception")
            {
                if (DotnetObjectId.TryParse(objRet?["value"]?["objectId"]?.Value<string>(), out DotnetObjectId objectId))
                {
                    var exceptionObject = await context.SdbAgent.GetObjectValues(int.Parse(objectId.Value), GetObjectCommandOptions.WithProperties | GetObjectCommandOptions.OwnProperties, token);
                    var exceptionObjectMessage = exceptionObject.FirstOrDefault(attr => attr["name"].Value<string>().Equals("_message"));
                    exceptionObjectMessage["value"]["value"] = objRet["value"]?["className"]?.Value<string>() + ": " + exceptionObjectMessage["value"]?["value"]?.Value<string>();
                    return exceptionObjectMessage["value"]?.Value<JObject>();
                }
                return objRet["value"]?.Value<JObject>();
            }

            if (objRet["value"]?.Value<JObject>() != null)
                return objRet["value"]?.Value<JObject>();
            if (objRet["get"]?.Value<JObject>() != null)
            {
                if (DotnetObjectId.TryParse(objRet?["get"]?["objectIdValue"]?.Value<string>(), out DotnetObjectId objectId))
                {
                    using var commandParamsWriter = new MonoBinaryWriter();
                    commandParamsWriter.WriteObj(objectId, context.SdbAgent);
                    var ret = await context.SdbAgent.InvokeMethod(commandParamsWriter.GetParameterBuffer(), objRet["get"]["methodId"].Value<int>(), objRet["name"].Value<string>(), token);
                    return await GetValueFromObject(ret, token);
                }

            }
            return null;
        }

        public async Task<JObject> TryToRunOnLoadedClasses(string varName, CancellationToken token)
        {
            string classNameToFind = "";
            string[] parts = varName.Split(".");
            var typeId = -1;
            foreach (string part in parts)
            {
                if (classNameToFind.Length > 0)
                    classNameToFind += ".";
                classNameToFind += part.Trim();
                if (typeId != -1)
                {
                    var fields = await context.SdbAgent.GetTypeFields(typeId, token);
                    foreach (var field in fields)
                    {
                        if (field.Name == part.Trim())
                        {
                            var isInitialized = await context.SdbAgent.TypeIsInitialized(typeId, token);
                            if (isInitialized == 0)
                            {
                                isInitialized = await context.SdbAgent.TypeInitialize(typeId, token);
                            }
                            var valueRet = await context.SdbAgent.GetFieldValue(typeId, field.Id, token);
                            return await GetValueFromObject(valueRet, token);
                        }
                    }
                    var methodId = await context.SdbAgent.GetPropertyMethodIdByName(typeId, part.Trim(), token);
                    if (methodId != -1)
                    {
                        using var commandParamsObjWriter = new MonoBinaryWriter();
                        commandParamsObjWriter.Write(0); //param count
                        var retMethod = await context.SdbAgent.InvokeMethod(commandParamsObjWriter.GetParameterBuffer(), methodId, "methodRet", token);
                        return await GetValueFromObject(retMethod, token);
                    }
                }
                var store = await proxy.LoadStore(sessionId, token);
                var methodInfo = context.CallStack.FirstOrDefault(s => s.Id == scopeId)?.Method?.Info;
                var classNameToFindWithNamespace =
                    string.IsNullOrEmpty(methodInfo?.TypeInfo?.Namespace) ?
                    classNameToFind :
                    methodInfo.TypeInfo.Namespace + "." + classNameToFind;

                var searchResult = await TryFindNameInAssembly(store.assemblies, classNameToFindWithNamespace);
                if (searchResult == null)
                    searchResult = await TryFindNameInAssembly(store.assemblies, classNameToFind);
                if (searchResult != null)
                    typeId = (int)searchResult;

                async Task<int?> TryGetTypeIdFromName(string typeName, AssemblyInfo assembly)
                {
                    var type = assembly.GetTypeByName(typeName);
                    if (type == null)
                        return null;
                    return await context.SdbAgent.GetTypeIdFromToken(assembly.DebugId, type.Token, token);
                }

                async Task<int?> TryFindNameInAssembly(List<AssemblyInfo> assemblies, string name)
                {
                    foreach (var asm in assemblies)
                    {
                        var typeId = await TryGetTypeIdFromName(name, asm);
                        if (typeId != null)
                            return typeId;
                    }
                    return null;
                }
            }
            return null;
        }

        // Checks Locals, followed by `this`
        public async Task<JObject> Resolve(string varName, CancellationToken token)
        {
            //has method calls
            if (varName.Contains('('))
                return null;

            string[] parts = varName.Split(".");
            JObject rootObject = null;

            if (scopeCache.MemberReferences.TryGetValue(varName, out JObject ret)) {
                return ret;
            }

            if (scopeCache.ObjectFields.TryGetValue(varName, out JObject valueRet)) {
                return await GetValueFromObject(valueRet, token);
            }

            foreach (string part in parts)
            {
                string partTrimmed = part.Trim();
                if (partTrimmed == "")
                    return null;
                if (rootObject != null)
                {
                    if (rootObject?["subtype"]?.Value<string>() == "null")
                        return null;
                    if (DotnetObjectId.TryParse(rootObject?["objectId"]?.Value<string>(), out DotnetObjectId objectId))
                    {
                        var rootResObj = await proxy.RuntimeGetPropertiesInternal(sessionId, objectId, null, token);
                        var objRet = rootResObj.FirstOrDefault(objPropAttr => objPropAttr["name"].Value<string>() == partTrimmed);
                        if (objRet == null)
                            return null;

                        rootObject = await GetValueFromObject(objRet, token);
                    }
                    continue;
                }
                if (scopeCache.Locals.Count == 0 && !localsFetched)
                {
                    Result scope_res = await proxy.GetScopeProperties(sessionId, scopeId, token);
                    if (scope_res.IsErr)
                        throw new Exception($"BUG: Unable to get properties for scope: {scopeId}. {scope_res}");
                    localsFetched = true;
                }
                if (scopeCache.Locals.TryGetValue(partTrimmed, out JObject obj))
                {
                    rootObject = obj["value"]?.Value<JObject>();
                }
                else if (scopeCache.Locals.TryGetValue("this", out JObject objThis))
                {
                    if (partTrimmed == "this")
                    {
                        rootObject = objThis?["value"].Value<JObject>();
                    }
                    else if (DotnetObjectId.TryParse(objThis?["value"]?["objectId"]?.Value<string>(), out DotnetObjectId objectId))
                    {
                        var rootResObj = await proxy.RuntimeGetPropertiesInternal(sessionId, objectId, null, token);
                        var objRet = rootResObj.FirstOrDefault(objPropAttr => objPropAttr["name"].Value<string>() == partTrimmed);
                        if (objRet != null)
                        {
                            rootObject = await GetValueFromObject(objRet, token);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            if (rootObject == null)
                rootObject = await TryToRunOnLoadedClasses(varName, token);
            scopeCache.MemberReferences[varName] = rootObject;
            return rootObject;
        }

        public async Task<JObject> Resolve(ElementAccessExpressionSyntax elementAccess, Dictionary<string, JObject> memberAccessValues, JObject indexObject, CancellationToken token)
        {
            try
            {
                JObject rootObject = null;
                string elementAccessStrExpression = elementAccess.Expression.ToString();
                rootObject = await Resolve(elementAccessStrExpression, token);
                if (rootObject == null)
                {
                    rootObject = indexObject;
                    indexObject = null;
                }
                if (rootObject != null)
                {
                    string elementIdxStr;
                    int elementIdx = 0;
                    // x[1] or x[a] or x[a.b]
                    if (indexObject == null)
                    {
                        if (elementAccess.ArgumentList != null)
                        {
                            foreach (var arg in elementAccess.ArgumentList.Arguments)
                            {
                                // e.g. x[1]
                                if (arg.Expression is LiteralExpressionSyntax)
                                {
                                    var argParm = arg.Expression as LiteralExpressionSyntax;
                                    elementIdxStr = argParm.ToString();
                                    int.TryParse(elementIdxStr, out elementIdx);
                                }

                                // e.g. x[a] or x[a.b]
                                if (arg.Expression is IdentifierNameSyntax)
                                {
                                    var argParm = arg.Expression as IdentifierNameSyntax;

                                    // x[a.b]
                                    memberAccessValues.TryGetValue(argParm.Identifier.Text, out indexObject);

                                    // x[a]
                                    if (indexObject == null)
                                    {
                                        indexObject = await Resolve(argParm.Identifier.Text, token);
                                    }
                                    elementIdxStr = indexObject["value"].ToString();
                                    int.TryParse(elementIdxStr, out elementIdx);
                                }
                            }
                        }
                    }
                    // e.g. x[a[0]], x[a[b[1]]] etc.
                    else
                    {
                        elementIdxStr = indexObject["value"].ToString();
                        int.TryParse(elementIdxStr, out elementIdx);
                    }
                    if (elementIdx >= 0)
                    {
                        DotnetObjectId.TryParse(rootObject?["objectId"]?.Value<string>(), out DotnetObjectId objectId);
                        switch (objectId.Scheme)
                        {
                            case "array":
                                rootObject["value"] = await context.SdbAgent.GetArrayValues(int.Parse(objectId.Value), token);
                                return (JObject)rootObject["value"][elementIdx]["value"];
                            case "object":
                                var typeIds = await context.SdbAgent.GetTypeIdFromObject(int.Parse(objectId.Value), true, token);
                                int methodId = await context.SdbAgent.GetMethodIdByName(typeIds[0], "ToArray", token);
                                var commandParamsObjWriter = new MonoBinaryWriter();
                                commandParamsObjWriter.WriteObj(objectId, context.SdbAgent);
                                var toArrayRetMethod = await context.SdbAgent.InvokeMethod(commandParamsObjWriter.GetParameterBuffer(), methodId, elementAccess.Expression.ToString(), token);
                                rootObject = await GetValueFromObject(toArrayRetMethod, token);
                                DotnetObjectId.TryParse(rootObject?["objectId"]?.Value<string>(), out DotnetObjectId arrayObjectId);
                                rootObject["value"] = await context.SdbAgent.GetArrayValues(int.Parse(arrayObjectId.Value), token);
                                return (JObject)rootObject["value"][elementIdx]["value"];
                            default:
                                throw new InvalidOperationException($"Cannot apply indexing with [] to an expression of type '{objectId.Scheme}'");
                        }
                    }
                }
                return null;
            }
            catch (Exception ex)
            {
                var e = ex;
                throw new Exception($"Unable to evaluate method '{elementAccess}'");
            }
        }

        public async Task<JObject> Resolve(InvocationExpressionSyntax method, Dictionary<string, JObject> memberAccessValues, CancellationToken token)
        {
            var methodName = "";
            int isTryingLinq = 0;
            try
            {
                JObject rootObject = null;
                var expr = method.Expression;
                if (expr is MemberAccessExpressionSyntax)
                {
                    var memberAccessExpressionSyntax = expr as MemberAccessExpressionSyntax;
                    rootObject = await Resolve(memberAccessExpressionSyntax.Expression.ToString(), token);
                    methodName = memberAccessExpressionSyntax.Name.ToString();
                }
                else if (expr is IdentifierNameSyntax)
                    if (scopeCache.ObjectFields.TryGetValue("this", out JObject valueRet)) {
                        rootObject = await GetValueFromObject(valueRet, token);
                    methodName = expr.ToString();
                }

                if (rootObject != null)
                {
                    DotnetObjectId.TryParse(rootObject?["objectId"]?.Value<string>(), out DotnetObjectId objectId);
                    var typeIds = await context.SdbAgent.GetTypeIdFromObject(int.Parse(objectId.Value), true, token);
                    int methodId = await context.SdbAgent.GetMethodIdByName(typeIds[0], methodName, token);
                    var className = await context.SdbAgent.GetTypeNameOriginal(typeIds[0], token);
                    if (methodId == 0) //try to search on System.Linq.Enumerable
                    {
                        if (linqTypeId == -1)
                            linqTypeId = await context.SdbAgent.GetTypeByName("System.Linq.Enumerable", token);
                        methodId = await context.SdbAgent.GetMethodIdByName(linqTypeId, methodName, token);
                        if (methodId != 0)
                        {
                            foreach (var typeId in typeIds)
                            {
                                var genericTypeArgs = await context.SdbAgent.GetTypeParamsOrArgsForGenericType(typeId, token);
                                if (genericTypeArgs.Count > 0)
                                {
                                    isTryingLinq = 1;
                                    methodId = await context.SdbAgent.MakeGenericMethod(methodId, genericTypeArgs, token);
                                    break;
                                }
                            }
                        }
                    }
                    if (methodId == 0) {
                        var typeName = await context.SdbAgent.GetTypeName(typeIds[0], token);
                        throw new Exception($"Method '{methodName}' not found in type '{typeName}'");
                    }
                    using var commandParamsObjWriter = new MonoBinaryWriter();
                    if (isTryingLinq == 0)
                        commandParamsObjWriter.WriteObj(objectId, context.SdbAgent);
                    if (method.ArgumentList != null)
                    {
                        commandParamsObjWriter.Write((int)method.ArgumentList.Arguments.Count + isTryingLinq);
                        if (isTryingLinq == 1)
                            commandParamsObjWriter.WriteObj(objectId, context.SdbAgent);
                        foreach (var arg in method.ArgumentList.Arguments)
                        {
                            if (arg.Expression is LiteralExpressionSyntax)
                            {
                                if (!await commandParamsObjWriter.WriteConst(arg.Expression as LiteralExpressionSyntax, context.SdbAgent, token))
                                    return null;
                            }
                            if (arg.Expression is IdentifierNameSyntax)
                            {
                                var argParm = arg.Expression as IdentifierNameSyntax;
                                if (!await commandParamsObjWriter.WriteJsonValue(memberAccessValues[argParm.Identifier.Text], context.SdbAgent, token))
                                    return null;
                            }
                        }
                        var retMethod = await context.SdbAgent.InvokeMethod(commandParamsObjWriter.GetParameterBuffer(), methodId, "methodRet", token);
                        return await GetValueFromObject(retMethod, token);
                    }
                }
                return null;
            }
            catch (Exception)
            {
                throw new Exception($"Unable to evaluate method '{methodName}'");
            }
        }
    }
}
