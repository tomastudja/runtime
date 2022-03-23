﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace Microsoft.Interop {
    using System;


    /// <summary>
    ///   A strongly-typed resource class, for looking up localized strings, etc.
    /// </summary>
    // This class was auto-generated by the StronglyTypedResourceBuilder
    // class via a tool like ResGen or Visual Studio.
    // To add or remove a member, edit your .ResX file then rerun ResGen
    // with the /str option, or rebuild your VS project.
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("System.Resources.Tools.StronglyTypedResourceBuilder", "17.0.0.0")]
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    internal class Resources {

        private static global::System.Resources.ResourceManager resourceMan;

        private static global::System.Globalization.CultureInfo resourceCulture;

        [global::System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        internal Resources() {
        }

        /// <summary>
        ///   Returns the cached ResourceManager instance used by this class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Resources.ResourceManager ResourceManager {
            get {
                if (object.ReferenceEquals(resourceMan, null)) {
                    global::System.Resources.ResourceManager temp = new global::System.Resources.ResourceManager("Microsoft.Interop.Resources", typeof(Resources).Assembly);
                    resourceMan = temp;
                }
                return resourceMan;
            }
        }

        /// <summary>
        ///   Overrides the current thread's CurrentUICulture property for all
        ///   resource lookups using this strongly typed resource class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Globalization.CultureInfo Culture {
            get {
                return resourceCulture;
            }
            set {
                resourceCulture = value;
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Marshalling an array from unmanaged to managed requires either the &apos;SizeParamIndex&apos; or &apos;SizeConst&apos; fields to be set on a &apos;MarshalAsAttribute&apos; or the &apos;ConstantElementCount&apos; or &apos;CountElementName&apos; properties to be set on a &apos;MarshalUsingAttribute&apos;..
        /// </summary>
        internal static string ArraySizeMustBeSpecified {
            get {
                return ResourceManager.GetString("ArraySizeMustBeSpecified", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;SizeParamIndex&apos; value in the &apos;MarshalAsAttribute&apos; is out of range..
        /// </summary>
        internal static string ArraySizeParamIndexOutOfRange {
            get {
                return ResourceManager.GetString("ArraySizeParamIndexOutOfRange", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;BlittableTypeAttribute&apos; and &apos;NativeMarshallingAttribute&apos; attributes are mutually exclusive..
        /// </summary>
        internal static string CannotHaveMultipleMarshallingAttributesDescription {
            get {
                return ResourceManager.GetString("CannotHaveMultipleMarshallingAttributesDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Type &apos;{0}&apos; is marked with &apos;BlittableTypeAttribute&apos; and &apos;NativeMarshallingAttribute&apos;. A type can only have one of these two attributes..
        /// </summary>
        internal static string CannotHaveMultipleMarshallingAttributesMessage {
            get {
                return ResourceManager.GetString("CannotHaveMultipleMarshallingAttributesMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to A native type with the &apos;GenericContiguousCollectionMarshallerAttribute&apos; must have at least one of the two marshalling methods as well as a &apos;ManagedValues&apos; property of type &apos;Span&lt;T&gt;&apos; for some &apos;T&apos; and a &apos;NativeValueStorage&apos; property of type &apos;Span&lt;byte&gt;&apos; to enable marshalling the managed type..
        /// </summary>
        internal static string CollectionNativeTypeMustHaveRequiredShapeDescription {
            get {
                return ResourceManager.GetString("CollectionNativeTypeMustHaveRequiredShapeDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The native type &apos;{0}&apos; must be a value type and have a constructor that takes two parameters, one of type &apos;{1}&apos; and an &apos;int&apos;, or have a parameterless instance method named &apos;ToManaged&apos; that returns &apos;{1}&apos; as well as a &apos;ManagedValues&apos; property of type &apos;Span&lt;T&gt;&apos; for some &apos;T&apos; and a &apos;NativeValueStorage&apos; property of type &apos;Span&lt;byte&gt;&apos;.
        /// </summary>
        internal static string CollectionNativeTypeMustHaveRequiredShapeMessage {
            get {
                return ResourceManager.GetString("CollectionNativeTypeMustHaveRequiredShapeMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The specified collection size parameter for an collection must be an integer type. If the size information is applied to a nested collection, the size parameter must be a collection of one less level of nesting with an integral element..
        /// </summary>
        internal static string CollectionSizeParamTypeMustBeIntegral {
            get {
                return ResourceManager.GetString("CollectionSizeParamTypeMustBeIntegral", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Only one of &apos;ConstantElementCount&apos; or &apos;ElementCountInfo&apos; may be used in a &apos;MarshalUsingAttribute&apos; for a given &apos;ElementIndirectionDepth&apos;.
        /// </summary>
        internal static string ConstantAndElementCountInfoDisallowed {
            get {
                return ResourceManager.GetString("ConstantAndElementCountInfoDisallowed", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The specified parameter needs to be marshalled from managed to native, but the native type &apos;{0}&apos; does not support it..
        /// </summary>
        internal static string CustomTypeMarshallingManagedToNativeUnsupported {
            get {
                return ResourceManager.GetString("CustomTypeMarshallingManagedToNativeUnsupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The specified parameter needs to be marshalled from native to managed, but the native type &apos;{0}&apos; does not support it..
        /// </summary>
        internal static string CustomTypeMarshallingNativeToManagedUnsupported {
            get {
                return ResourceManager.GetString("CustomTypeMarshallingNativeToManagedUnsupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to This element cannot depend on &apos;{0}&apos; for collection size information without creating a dependency cycle.
        /// </summary>
        internal static string CyclicalCountInfo {
            get {
                return ResourceManager.GetString("CyclicalCountInfo", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Count information for a given element at a given indirection level can only be specified once.
        /// </summary>
        internal static string DuplicateCountInfo {
            get {
                return ResourceManager.GetString("DuplicateCountInfo", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Multiple marshalling attributes per element per indirection level is unsupported, but duplicate information was provided for indirection level {0}.
        /// </summary>
        internal static string DuplicateMarshallingInfo {
            get {
                return ResourceManager.GetString("DuplicateMarshallingInfo", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Marshalling info was specified for &apos;ElementIndirectionDepth&apos; {0}, but marshalling info was only needed for {1} level(s) of indirection.
        /// </summary>
        internal static string ExtraneousMarshallingInfo {
            get {
                return ResourceManager.GetString("ExtraneousMarshallingInfo", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The provided graph has cycles and cannot be topologically sorted..
        /// </summary>
        internal static string GraphHasCycles {
            get {
                return ResourceManager.GetString("GraphHasCycles", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;[In]&apos; attribute is not supported unless the &apos;[Out]&apos; attribute is also used. The behavior of the &apos;[In]&apos; attribute without the &apos;[Out]&apos; attribute is the same as the default behavior..
        /// </summary>
        internal static string InAttributeNotSupportedWithoutOut {
            get {
                return ResourceManager.GetString("InAttributeNotSupportedWithoutOut", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;[In]&apos; and &apos;[Out]&apos; attributes are unsupported on parameters passed by reference. Use the &apos;in&apos;, &apos;ref&apos;, or &apos;out&apos; keywords instead..
        /// </summary>
        internal static string InOutAttributeByRefNotSupported {
            get {
                return ResourceManager.GetString("InOutAttributeByRefNotSupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The provided &apos;[In]&apos; and &apos;[Out]&apos; attributes on this parameter are unsupported on this parameter..
        /// </summary>
        internal static string InOutAttributeMarshalerNotSupported {
            get {
                return ResourceManager.GetString("InOutAttributeMarshalerNotSupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Marshalling bool without explicit marshalling information is not supported. Specify either &apos;MarshalUsingAttribute&apos; or &apos;MarshalAsAttribute&apos;..
        /// </summary>
        internal static string MarshallingBoolAsUndefinedNotSupported {
            get {
                return ResourceManager.GetString("MarshallingBoolAsUndefinedNotSupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Marshalling char with &apos;CharSet.{0}&apos; is not supported. Instead, manually convert the char type to the desired byte representation and pass to the source-generated P/Invoke..
        /// </summary>
        internal static string MarshallingCharAsSpecifiedCharSetNotSupported {
            get {
                return ResourceManager.GetString("MarshallingCharAsSpecifiedCharSetNotSupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Marshalling string or char without explicit marshalling information is not supported. Specify either &apos;LibraryImportAttribute.CharSet&apos; or &apos;MarshalAsAttribute&apos;..
        /// </summary>
        internal static string MarshallingStringOrCharAsUndefinedNotSupported {
            get {
                return ResourceManager.GetString("MarshallingStringOrCharAsUndefinedNotSupported", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The native type &apos;{0}&apos; for managed type &apos;{1}&apos; must be a closed generic type or have the same arity as the managed type..
        /// </summary>
        internal static string NativeGenericTypeMustBeClosedOrMatchArityMessage {
            get {
                return ResourceManager.GetString("NativeGenericTypeMustBeClosedOrMatchArityMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The native type must have at least one of the two marshalling methods to enable marshalling the managed type..
        /// </summary>
        internal static string NativeTypeMustHaveRequiredShapeDescription {
            get {
                return ResourceManager.GetString("NativeTypeMustHaveRequiredShapeDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The native type &apos;{0}&apos; must be a value type and have a constructor that takes one parameter of type &apos;{1}&apos; or a parameterless instance method named &apos;ToManaged&apos; that returns &apos;{1}&apos;.
        /// </summary>
        internal static string NativeTypeMustHaveRequiredShapeMessage {
            get {
                return ResourceManager.GetString("NativeTypeMustHaveRequiredShapeMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;[Out]&apos; attribute is only supported on array parameters..
        /// </summary>
        internal static string OutByValueNotSupportedDescription {
            get {
                return ResourceManager.GetString("OutByValueNotSupportedDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;[Out]&apos; attribute is not supported on the &apos;{0}&apos; parameter..
        /// </summary>
        internal static string OutByValueNotSupportedMessage {
            get {
                return ResourceManager.GetString("OutByValueNotSupportedMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;Value&apos; property must not be a &apos;ref&apos; or &apos;readonly ref&apos; property..
        /// </summary>
        internal static string RefNativeValueUnsupportedDescription {
            get {
                return ResourceManager.GetString("RefNativeValueUnsupportedDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;Value&apos; property on the native type &apos;{0}&apos; must not be a &apos;ref&apos; or &apos;readonly ref&apos; property..
        /// </summary>
        internal static string RefNativeValueUnsupportedMessage {
            get {
                return ResourceManager.GetString("RefNativeValueUnsupportedMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Runtime marshalling must be disabled in this project by applying the &apos;System.Runtime.InteropServices.DisableRuntimeMarshallingAttribute&apos; to the assembly to enable marshalling this type..
        /// </summary>
        internal static string RuntimeMarshallingMustBeDisabled {
            get {
                return ResourceManager.GetString("RuntimeMarshallingMustBeDisabled", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to An abstract type derived from &apos;SafeHandle&apos; cannot be marshalled by reference. The provided type must be concrete..
        /// </summary>
        internal static string SafeHandleByRefMustBeConcrete {
            get {
                return ResourceManager.GetString("SafeHandleByRefMustBeConcrete", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Specified type is not supported by source-generated P/Invokes.
        /// </summary>
        internal static string TypeNotSupportedTitle {
            get {
                return ResourceManager.GetString("TypeNotSupportedTitle", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to Marshalling a value between managed and native with a native type with a &apos;Value&apos; property requires extra state, which is not supported in this context..
        /// </summary>
        internal static string ValuePropertyMarshallingRequiresAdditionalState {
            get {
                return ResourceManager.GetString("ValuePropertyMarshallingRequiresAdditionalState", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The native type&apos;s &apos;Value&apos; property must have a getter to support marshalling from managed to native..
        /// </summary>
        internal static string ValuePropertyMustHaveGetterDescription {
            get {
                return ResourceManager.GetString("ValuePropertyMustHaveGetterDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;Value&apos; property on the native type &apos;{0}&apos; must have a getter.
        /// </summary>
        internal static string ValuePropertyMustHaveGetterMessage {
            get {
                return ResourceManager.GetString("ValuePropertyMustHaveGetterMessage", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The native type&apos;s &apos;Value&apos; property must have a setter to support marshalling from native to managed..
        /// </summary>
        internal static string ValuePropertyMustHaveSetterDescription {
            get {
                return ResourceManager.GetString("ValuePropertyMustHaveSetterDescription", resourceCulture);
            }
        }

        /// <summary>
        ///   Looks up a localized string similar to The &apos;Value&apos; property on the native type &apos;{0}&apos; must have a setter.
        /// </summary>
        internal static string ValuePropertyMustHaveSetterMessage {
            get {
                return ResourceManager.GetString("ValuePropertyMustHaveSetterMessage", resourceCulture);
            }
        }
    }
}
