/**
 * \file
 * Class management for the Mono runtime
 *
 * Author:
 *   Miguel de Icaza (miguel@ximian.com)
 *
 * Copyright 2001-2003 Ximian, Inc (http://www.ximian.com)
 * Copyright 2004-2009 Novell, Inc (http://www.novell.com)
 * Copyright 2012 Xamarin Inc (http://www.xamarin.com)
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#include <config.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mono/metadata/image.h>
#include <mono/metadata/image-internals.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/assembly-internals.h>
#include <mono/metadata/exception-internals.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/metadata-internals.h>
#include <mono/metadata/profiler-private.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/class-init.h>
#include <mono/metadata/class-internals.h>
#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/mono-endian.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/security-manager.h>
#include <mono/metadata/security-core-clr.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/gc-internals.h>
#include <mono/metadata/verify-internals.h>
#include <mono/metadata/mono-debug.h>
#include <mono/utils/mono-counters.h>
#include <mono/utils/mono-string.h>
#include <mono/utils/mono-error-internals.h>
#include <mono/utils/mono-logger-internals.h>
#include <mono/utils/mono-memory-model.h>
#include <mono/utils/atomic.h>
#include <mono/utils/unlocked.h>
#include <mono/utils/bsearch.h>
#include <mono/utils/checked-build.h>

MonoStats mono_stats;

/* Statistics */
extern gint32 inflated_methods_size;

/* Function supplied by the runtime to find classes by name using information from the AOT file */
static MonoGetClassFromName get_class_from_name = NULL;

static gboolean can_access_type (MonoClass *access_klass, MonoClass *member_klass);

static char* mono_assembly_name_from_token (MonoImage *image, guint32 type_token);
static guint32 mono_field_resolve_flags (MonoClassField *field);

/**
 * mono_class_from_typeref:
 * \param image a MonoImage
 * \param type_token a TypeRef token
 *
 * Creates the \c MonoClass* structure representing the type defined by
 * the typeref token valid inside \p image.
 * \returns The \c MonoClass* representing the typeref token, or NULL if it could
 * not be loaded.
 */
MonoClass *
mono_class_from_typeref (MonoImage *image, guint32 type_token)
{
	ERROR_DECL (error);
	MonoClass *klass = mono_class_from_typeref_checked (image, type_token, error);
	g_assert (mono_error_ok (error)); /*FIXME proper error handling*/
	return klass;
}

/**
 * mono_class_from_typeref_checked:
 * \param image a MonoImage
 * \param type_token a TypeRef token
 * \param error error return code, if any.
 *
 * Creates the \c MonoClass* structure representing the type defined by
 * the typeref token valid inside \p image.
 *
 * \returns The \c MonoClass* representing the typeref token, NULL if it could
 * not be loaded with the \p error value filled with the information about the
 * error.
 */
MonoClass *
mono_class_from_typeref_checked (MonoImage *image, guint32 type_token, MonoError *error)
{
	guint32 cols [MONO_TYPEREF_SIZE];
	MonoTableInfo  *t = &image->tables [MONO_TABLE_TYPEREF];
	guint32 idx;
	const char *name, *nspace;
	MonoClass *res = NULL;
	MonoImage *module;

	error_init (error);

	if (!mono_verifier_verify_typeref_row (image, (type_token & 0xffffff) - 1, error))
		return NULL;

	mono_metadata_decode_row (t, (type_token&0xffffff)-1, cols, MONO_TYPEREF_SIZE);

	name = mono_metadata_string_heap (image, cols [MONO_TYPEREF_NAME]);
	nspace = mono_metadata_string_heap (image, cols [MONO_TYPEREF_NAMESPACE]);

	idx = cols [MONO_TYPEREF_SCOPE] >> MONO_RESOLUTION_SCOPE_BITS;
	switch (cols [MONO_TYPEREF_SCOPE] & MONO_RESOLUTION_SCOPE_MASK) {
	case MONO_RESOLUTION_SCOPE_MODULE:
		/*
		LAMESPEC The spec says that a null module resolution scope should go through the exported type table.
		This is not the observed behavior of existing implementations.
		The defacto behavior is that it's just a typedef in disguise.
		*/
		/* a typedef in disguise */
		res = mono_class_from_name_checked (image, nspace, name, error);
		goto done;

	case MONO_RESOLUTION_SCOPE_MODULEREF:
		module = mono_image_load_module_checked (image, idx, error);
		if (module)
			res = mono_class_from_name_checked (module, nspace, name, error);
		goto done;

	case MONO_RESOLUTION_SCOPE_TYPEREF: {
		MonoClass *enclosing;
		GList *tmp;

		if (idx == mono_metadata_token_index (type_token)) {
			mono_error_set_bad_image (error, image, "Image with self-referencing typeref token %08x.", type_token);
			return NULL;
		}

		enclosing = mono_class_from_typeref_checked (image, MONO_TOKEN_TYPE_REF | idx, error); 
		return_val_if_nok (error, NULL);

		GList *nested_classes = mono_class_get_nested_classes_property (enclosing);
		if (enclosing->nested_classes_inited && nested_classes) {
			/* Micro-optimization: don't scan the metadata tables if enclosing is already inited */
			for (tmp = nested_classes; tmp; tmp = tmp->next) {
				res = (MonoClass *)tmp->data;
				if (strcmp (res->name, name) == 0)
					return res;
			}
		} else {
			/* Don't call mono_class_init as we might've been called by it recursively */
			int i = mono_metadata_nesting_typedef (enclosing->image, enclosing->type_token, 1);
			while (i) {
				guint32 class_nested = mono_metadata_decode_row_col (&enclosing->image->tables [MONO_TABLE_NESTEDCLASS], i - 1, MONO_NESTED_CLASS_NESTED);
				guint32 string_offset = mono_metadata_decode_row_col (&enclosing->image->tables [MONO_TABLE_TYPEDEF], class_nested - 1, MONO_TYPEDEF_NAME);
				const char *nname = mono_metadata_string_heap (enclosing->image, string_offset);

				if (strcmp (nname, name) == 0)
					return mono_class_create_from_typedef (enclosing->image, MONO_TOKEN_TYPE_DEF | class_nested, error);

				i = mono_metadata_nesting_typedef (enclosing->image, enclosing->type_token, i + 1);
			}
		}
		g_warning ("TypeRef ResolutionScope not yet handled (%d) for %s.%s in image %s", idx, nspace, name, image->name);
		goto done;
	}
	case MONO_RESOLUTION_SCOPE_ASSEMBLYREF:
		break;
	}

	if (idx > image->tables [MONO_TABLE_ASSEMBLYREF].rows) {
		mono_error_set_bad_image (error, image, "Image with invalid assemblyref token %08x.", idx);
		return NULL;
	}

	if (!image->references || !image->references [idx - 1])
		mono_assembly_load_reference (image, idx - 1);
	g_assert (image->references [idx - 1]);

	/* If the assembly did not load, register this as a type load exception */
	if (image->references [idx - 1] == REFERENCE_MISSING){
		MonoAssemblyName aname;
		char *human_name;
		
		mono_assembly_get_assemblyref (image, idx - 1, &aname);
		human_name = mono_stringify_assembly_name (&aname);
		mono_error_set_simple_file_not_found (error, human_name, image->assembly ? image->assembly->ref_only : FALSE);
		g_free (human_name);
		return NULL;
	}

	res = mono_class_from_name_checked (image->references [idx - 1]->image, nspace, name, error);

done:
	/* Generic case, should be avoided for when a better error is possible. */
	if (!res && mono_error_ok (error)) {
		char *name = mono_class_name_from_token (image, type_token);
		char *assembly = mono_assembly_name_from_token (image, type_token);
		mono_error_set_type_load_name (error, name, assembly, "Could not resolve type with token %08x from typeref (expected class '%s' in assembly '%s')", type_token, name, assembly);
	}
	return res;
}


static void *
mono_image_memdup (MonoImage *image, void *data, guint size)
{
	void *res = mono_image_alloc (image, size);
	memcpy (res, data, size);
	return res;
}
	
/* Copy everything mono_metadata_free_array free. */
MonoArrayType *
mono_dup_array_type (MonoImage *image, MonoArrayType *a)
{
	if (image) {
		a = (MonoArrayType *)mono_image_memdup (image, a, sizeof (MonoArrayType));
		if (a->sizes)
			a->sizes = (int *)mono_image_memdup (image, a->sizes, a->numsizes * sizeof (int));
		if (a->lobounds)
			a->lobounds = (int *)mono_image_memdup (image, a->lobounds, a->numlobounds * sizeof (int));
	} else {
		a = (MonoArrayType *)g_memdup (a, sizeof (MonoArrayType));
		if (a->sizes)
			a->sizes = (int *)g_memdup (a->sizes, a->numsizes * sizeof (int));
		if (a->lobounds)
			a->lobounds = (int *)g_memdup (a->lobounds, a->numlobounds * sizeof (int));
	}
	return a;
}

/* Copy everything mono_metadata_free_method_signature free. */
MonoMethodSignature*
mono_metadata_signature_deep_dup (MonoImage *image, MonoMethodSignature *sig)
{
	int i;
	
	sig = mono_metadata_signature_dup_full (image, sig);
	
	sig->ret = mono_metadata_type_dup (image, sig->ret);
	for (i = 0; i < sig->param_count; ++i)
		sig->params [i] = mono_metadata_type_dup (image, sig->params [i]);
	
	return sig;
}

static void
_mono_type_get_assembly_name (MonoClass *klass, GString *str)
{
	MonoAssembly *ta = klass->image->assembly;
	char *name;

	name = mono_stringify_assembly_name (&ta->aname);
	g_string_append_printf (str, ", %s", name);
	g_free (name);
}

static inline void
mono_type_name_check_byref (MonoType *type, GString *str)
{
	if (type->byref)
		g_string_append_c (str, '&');
}

/**
 * mono_identifier_escape_type_name_chars:
 * \param str a destination string
 * \param identifier an IDENTIFIER in internal form
 *
 * \returns \p str
 *
 * The displayed form of the identifier is appended to str.
 *
 * The displayed form of an identifier has the characters ,+&*[]\
 * that have special meaning in type names escaped with a preceeding
 * backslash (\) character.
 */
static GString*
mono_identifier_escape_type_name_chars (GString* str, const char* identifier)
{
	if (!identifier)
		return str;

	size_t n = str->len;
	// reserve space for common case: there will be no escaped characters.
	g_string_set_size(str, n + strlen(identifier));
	g_string_set_size(str, n);

	for (const char* s = identifier; *s != 0 ; s++) {
		switch (*s) {
		case ',':
		case '+':
		case '&':
		case '*':
		case '[':
		case ']':
		case '\\':
			g_string_append_c (str, '\\');
			g_string_append_c (str, *s);
			break;
		default:
			g_string_append_c (str, *s);
			break;
		}
	}
	return str;
}

static void
mono_type_get_name_recurse (MonoType *type, GString *str, gboolean is_recursed,
			    MonoTypeNameFormat format)
{
	MonoClass *klass;
	
	switch (type->type) {
	case MONO_TYPE_ARRAY: {
		int i, rank = type->data.array->rank;
		MonoTypeNameFormat nested_format;

		nested_format = format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED ?
			MONO_TYPE_NAME_FORMAT_FULL_NAME : format;

		mono_type_get_name_recurse (
			&type->data.array->eklass->byval_arg, str, FALSE, nested_format);
		g_string_append_c (str, '[');
		if (rank == 1)
			g_string_append_c (str, '*');
		for (i = 1; i < rank; i++)
			g_string_append_c (str, ',');
		g_string_append_c (str, ']');
		
		mono_type_name_check_byref (type, str);

		if (format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED)
			_mono_type_get_assembly_name (type->data.array->eklass, str);
		break;
	}
	case MONO_TYPE_SZARRAY: {
		MonoTypeNameFormat nested_format;

		nested_format = format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED ?
			MONO_TYPE_NAME_FORMAT_FULL_NAME : format;

		mono_type_get_name_recurse (
			&type->data.klass->byval_arg, str, FALSE, nested_format);
		g_string_append (str, "[]");
		
		mono_type_name_check_byref (type, str);

		if (format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED)
			_mono_type_get_assembly_name (type->data.klass, str);
		break;
	}
	case MONO_TYPE_PTR: {
		MonoTypeNameFormat nested_format;

		nested_format = format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED ?
			MONO_TYPE_NAME_FORMAT_FULL_NAME : format;

		mono_type_get_name_recurse (
			type->data.type, str, FALSE, nested_format);
		g_string_append_c (str, '*');

		mono_type_name_check_byref (type, str);

		if (format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED)
			_mono_type_get_assembly_name (mono_class_from_mono_type (type->data.type), str);
		break;
	}
	case MONO_TYPE_VAR:
	case MONO_TYPE_MVAR:
		if (!mono_generic_param_info (type->data.generic_param))
			g_string_append_printf (str, "%s%d", type->type == MONO_TYPE_VAR ? "!" : "!!", type->data.generic_param->num);
		else
			g_string_append (str, mono_generic_param_info (type->data.generic_param)->name);

		mono_type_name_check_byref (type, str);

		break;
	default:
		klass = mono_class_from_mono_type (type);
		if (klass->nested_in) {
			mono_type_get_name_recurse (
				&klass->nested_in->byval_arg, str, TRUE, format);
			if (format == MONO_TYPE_NAME_FORMAT_IL)
				g_string_append_c (str, '.');
			else
				g_string_append_c (str, '+');
		} else if (*klass->name_space) {
			if (format == MONO_TYPE_NAME_FORMAT_IL)
				g_string_append (str, klass->name_space);
			else
				mono_identifier_escape_type_name_chars (str, klass->name_space);
			g_string_append_c (str, '.');
		}
		if (format == MONO_TYPE_NAME_FORMAT_IL) {
			char *s = strchr (klass->name, '`');
			int len = s ? s - klass->name : strlen (klass->name);
			g_string_append_len (str, klass->name, len);
		} else {
			mono_identifier_escape_type_name_chars (str, klass->name);
		}
		if (is_recursed)
			break;
		if (mono_class_is_ginst (klass)) {
			MonoGenericClass *gclass = mono_class_get_generic_class (klass);
			MonoGenericInst *inst = gclass->context.class_inst;
			MonoTypeNameFormat nested_format;
			int i;

			nested_format = format == MONO_TYPE_NAME_FORMAT_FULL_NAME ?
				MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED : format;

			if (format == MONO_TYPE_NAME_FORMAT_IL)
				g_string_append_c (str, '<');
			else
				g_string_append_c (str, '[');
			for (i = 0; i < inst->type_argc; i++) {
				MonoType *t = inst->type_argv [i];

				if (i)
					g_string_append_c (str, ',');
				if ((nested_format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED) &&
				    (t->type != MONO_TYPE_VAR) && (type->type != MONO_TYPE_MVAR))
					g_string_append_c (str, '[');
				mono_type_get_name_recurse (inst->type_argv [i], str, FALSE, nested_format);
				if ((nested_format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED) &&
				    (t->type != MONO_TYPE_VAR) && (type->type != MONO_TYPE_MVAR))
					g_string_append_c (str, ']');
			}
			if (format == MONO_TYPE_NAME_FORMAT_IL)	
				g_string_append_c (str, '>');
			else
				g_string_append_c (str, ']');
		} else if (mono_class_is_gtd (klass) &&
			   (format != MONO_TYPE_NAME_FORMAT_FULL_NAME) &&
			   (format != MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED)) {
			int i;

			if (format == MONO_TYPE_NAME_FORMAT_IL)	
				g_string_append_c (str, '<');
			else
				g_string_append_c (str, '[');
			for (i = 0; i < mono_class_get_generic_container (klass)->type_argc; i++) {
				if (i)
					g_string_append_c (str, ',');
				g_string_append (str, mono_generic_container_get_param_info (mono_class_get_generic_container (klass), i)->name);
			}
			if (format == MONO_TYPE_NAME_FORMAT_IL)	
				g_string_append_c (str, '>');
			else
				g_string_append_c (str, ']');
		}

		mono_type_name_check_byref (type, str);

		if ((format == MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED) &&
		    (type->type != MONO_TYPE_VAR) && (type->type != MONO_TYPE_MVAR))
			_mono_type_get_assembly_name (klass, str);
		break;
	}
}

/**
 * mono_type_get_name_full:
 * \param type a type
 * \param format the format for the return string.
 *
 * 
 * \returns The string representation in a number of formats:
 *
 * if \p format is \c MONO_TYPE_NAME_FORMAT_REFLECTION, the return string is
 * returned in the format required by \c System.Reflection, this is the
 * inverse of mono_reflection_parse_type().
 *
 * if \p format is \c MONO_TYPE_NAME_FORMAT_IL, it returns a syntax that can
 * be used by the IL assembler.
 *
 * if \p format is \c MONO_TYPE_NAME_FORMAT_FULL_NAME
 *
 * if \p format is \c MONO_TYPE_NAME_FORMAT_ASSEMBLY_QUALIFIED
 */
char*
mono_type_get_name_full (MonoType *type, MonoTypeNameFormat format)
{
	GString* result;

	result = g_string_new ("");

	mono_type_get_name_recurse (type, result, FALSE, format);

	return g_string_free (result, FALSE);
}

/**
 * mono_type_get_full_name:
 * \param class a class
 *
 * \returns The string representation for type as required by System.Reflection.
 * The inverse of mono_reflection_parse_type().
 */
char *
mono_type_get_full_name (MonoClass *klass)
{
	return mono_type_get_name_full (mono_class_get_type (klass), MONO_TYPE_NAME_FORMAT_REFLECTION);
}

/**
 * mono_type_get_name:
 * \param type a type
 * \returns The string representation for type as it would be represented in IL code.
 */
char*
mono_type_get_name (MonoType *type)
{
	return mono_type_get_name_full (type, MONO_TYPE_NAME_FORMAT_IL);
}

/**
 * mono_type_get_underlying_type:
 * \param type a type
 * \returns The \c MonoType for the underlying integer type if \p type
 * is an enum and byref is false, otherwise the type itself.
 */
MonoType*
mono_type_get_underlying_type (MonoType *type)
{
	if (type->type == MONO_TYPE_VALUETYPE && type->data.klass->enumtype && !type->byref)
		return mono_class_enum_basetype (type->data.klass);
	if (type->type == MONO_TYPE_GENERICINST && type->data.generic_class->container_class->enumtype && !type->byref)
		return mono_class_enum_basetype (type->data.generic_class->container_class);
	return type;
}

/**
 * mono_class_is_open_constructed_type:
 * \param type a type
 *
 * \returns TRUE if type represents a generics open constructed type.
 * IOW, not all type parameters required for the instantiation have
 * been provided or it's a generic type definition.
 *
 * An open constructed type means it's a non realizable type. Not to
 * be mixed up with an abstract type - we can't cast or dispatch to
 * an open type, for example.
 */
gboolean
mono_class_is_open_constructed_type (MonoType *t)
{
	switch (t->type) {
	case MONO_TYPE_VAR:
	case MONO_TYPE_MVAR:
		return TRUE;
	case MONO_TYPE_SZARRAY:
		return mono_class_is_open_constructed_type (&t->data.klass->byval_arg);
	case MONO_TYPE_ARRAY:
		return mono_class_is_open_constructed_type (&t->data.array->eklass->byval_arg);
	case MONO_TYPE_PTR:
		return mono_class_is_open_constructed_type (t->data.type);
	case MONO_TYPE_GENERICINST:
		return t->data.generic_class->context.class_inst->is_open;
	case MONO_TYPE_CLASS:
	case MONO_TYPE_VALUETYPE:
		return mono_class_is_gtd (t->data.klass);
	default:
		return FALSE;
	}
}

/*
This is a simple function to catch the most common bad instances of generic types.
Specially those that might lead to further failures in the runtime.
*/
static gboolean
is_valid_generic_argument (MonoType *type)
{
	switch (type->type) {
	case MONO_TYPE_VOID:
	//case MONO_TYPE_TYPEDBYREF:
		return FALSE;
	default:
		return TRUE;
	}
}

static MonoType*
inflate_generic_type (MonoImage *image, MonoType *type, MonoGenericContext *context, MonoError *error)
{
	error_init (error);

	switch (type->type) {
	case MONO_TYPE_MVAR: {
		MonoType *nt;
		int num = mono_type_get_generic_param_num (type);
		MonoGenericInst *inst = context->method_inst;
		if (!inst)
			return NULL;
		if (num >= inst->type_argc) {
			MonoGenericParamInfo *info = mono_generic_param_info (type->data.generic_param);
			mono_error_set_bad_image (error, image, "MVAR %d (%s) cannot be expanded in this context with %d instantiations",
				num, info ? info->name : "", inst->type_argc);
			return NULL;
		}

		if (!is_valid_generic_argument (inst->type_argv [num])) {
			MonoGenericParamInfo *info = mono_generic_param_info (type->data.generic_param);
			mono_error_set_bad_image (error, image, "MVAR %d (%s) cannot be expanded with type 0x%x",
				num, info ? info->name : "", inst->type_argv [num]->type);
			return NULL;			
		}
		/*
		 * Note that the VAR/MVAR cases are different from the rest.  The other cases duplicate @type,
		 * while the VAR/MVAR duplicates a type from the context.  So, we need to ensure that the
		 * ->byref and ->attrs from @type are propagated to the returned type.
		 */
		nt = mono_metadata_type_dup (image, inst->type_argv [num]);
		nt->byref = type->byref;
		nt->attrs = type->attrs;
		return nt;
	}
	case MONO_TYPE_VAR: {
		MonoType *nt;
		int num = mono_type_get_generic_param_num (type);
		MonoGenericInst *inst = context->class_inst;
		if (!inst)
			return NULL;
		if (num >= inst->type_argc) {
			MonoGenericParamInfo *info = mono_generic_param_info (type->data.generic_param);
			mono_error_set_bad_image (error, image, "VAR %d (%s) cannot be expanded in this context with %d instantiations",
				num, info ? info->name : "", inst->type_argc);
			return NULL;
		}
		if (!is_valid_generic_argument (inst->type_argv [num])) {
			MonoGenericParamInfo *info = mono_generic_param_info (type->data.generic_param);
			mono_error_set_bad_image (error, image, "VAR %d (%s) cannot be expanded with type 0x%x",
				num, info ? info->name : "", inst->type_argv [num]->type);
			return NULL;			
		}
		nt = mono_metadata_type_dup (image, inst->type_argv [num]);
		nt->byref = type->byref;
		nt->attrs = type->attrs;
		return nt;
	}
	case MONO_TYPE_SZARRAY: {
		MonoClass *eclass = type->data.klass;
		MonoType *nt, *inflated = inflate_generic_type (NULL, &eclass->byval_arg, context, error);
		if (!inflated || !mono_error_ok (error))
			return NULL;
		nt = mono_metadata_type_dup (image, type);
		nt->data.klass = mono_class_from_mono_type (inflated);
		mono_metadata_free_type (inflated);
		return nt;
	}
	case MONO_TYPE_ARRAY: {
		MonoClass *eclass = type->data.array->eklass;
		MonoType *nt, *inflated = inflate_generic_type (NULL, &eclass->byval_arg, context, error);
		if (!inflated || !mono_error_ok (error))
			return NULL;
		nt = mono_metadata_type_dup (image, type);
		nt->data.array->eklass = mono_class_from_mono_type (inflated);
		mono_metadata_free_type (inflated);
		return nt;
	}
	case MONO_TYPE_GENERICINST: {
		MonoGenericClass *gclass = type->data.generic_class;
		MonoGenericInst *inst;
		MonoType *nt;
		if (!gclass->context.class_inst->is_open)
			return NULL;

		inst = mono_metadata_inflate_generic_inst (gclass->context.class_inst, context, error);
		return_val_if_nok (error, NULL);

		if (inst != gclass->context.class_inst)
			gclass = mono_metadata_lookup_generic_class (gclass->container_class, inst, gclass->is_dynamic);

		if (gclass == type->data.generic_class)
			return NULL;

		nt = mono_metadata_type_dup (image, type);
		nt->data.generic_class = gclass;
		return nt;
	}
	case MONO_TYPE_CLASS:
	case MONO_TYPE_VALUETYPE: {
		MonoClass *klass = type->data.klass;
		MonoGenericContainer *container = mono_class_try_get_generic_container (klass);
		MonoGenericInst *inst;
		MonoGenericClass *gclass = NULL;
		MonoType *nt;

		if (!container)
			return NULL;

		/* We can't use context->class_inst directly, since it can have more elements */
		inst = mono_metadata_inflate_generic_inst (container->context.class_inst, context, error);
		return_val_if_nok (error, NULL);

		if (inst == container->context.class_inst)
			return NULL;

		gclass = mono_metadata_lookup_generic_class (klass, inst, image_is_dynamic (klass->image));

		nt = mono_metadata_type_dup (image, type);
		nt->type = MONO_TYPE_GENERICINST;
		nt->data.generic_class = gclass;
		return nt;
	}
	default:
		return NULL;
	}
	return NULL;
}

MonoGenericContext *
mono_generic_class_get_context (MonoGenericClass *gclass)
{
	return &gclass->context;
}

MonoGenericContext *
mono_class_get_context (MonoClass *klass)
{
	MonoGenericClass *gklass = mono_class_try_get_generic_class (klass);
	return gklass ? mono_generic_class_get_context (gklass) : NULL;
}

/*
 * mono_class_inflate_generic_type_with_mempool:
 * @mempool: a mempool
 * @type: a type
 * @context: a generics context
 * @error: error context
 *
 * The same as mono_class_inflate_generic_type, but allocates the MonoType
 * from mempool if it is non-NULL.  If it is NULL, the MonoType is
 * allocated on the heap and is owned by the caller.
 * The returned type can potentially be the same as TYPE, so it should not be
 * modified by the caller, and it should be freed using mono_metadata_free_type ().
 */
MonoType*
mono_class_inflate_generic_type_with_mempool (MonoImage *image, MonoType *type, MonoGenericContext *context, MonoError *error)
{
	MonoType *inflated = NULL;
	error_init (error);

	if (context)
		inflated = inflate_generic_type (image, type, context, error);
	return_val_if_nok (error, NULL);

	if (!inflated) {
		MonoType *shared = mono_metadata_get_shared_type (type);

		if (shared) {
			return shared;
		} else {
			return mono_metadata_type_dup (image, type);
		}
	}

	UnlockedIncrement (&mono_stats.inflated_type_count);
	return inflated;
}

/**
 * mono_class_inflate_generic_type:
 * \param type a type
 * \param context a generics context
 * \deprecated Please use \c mono_class_inflate_generic_type_checked instead
 *
 * If \p type is a generic type and \p context is not NULL, instantiate it using the 
 * generics context \p context.
 *
 * \returns The instantiated type or a copy of \p type. The returned \c MonoType is allocated
 * on the heap and is owned by the caller. Returns NULL on error.
 */
MonoType*
mono_class_inflate_generic_type (MonoType *type, MonoGenericContext *context)
{
	ERROR_DECL (error);
	MonoType *result;
	result = mono_class_inflate_generic_type_checked (type, context, error);
	mono_error_cleanup (error);
	return result;
}

/*
 * mono_class_inflate_generic_type:
 * @type: a type
 * @context: a generics context
 * @error: error context to use
 *
 * If @type is a generic type and @context is not NULL, instantiate it using the 
 * generics context @context.
 *
 * Returns: The instantiated type or a copy of @type. The returned MonoType is allocated
 * on the heap and is owned by the caller.
 */
MonoType*
mono_class_inflate_generic_type_checked (MonoType *type, MonoGenericContext *context, MonoError *error)
{
	return mono_class_inflate_generic_type_with_mempool (NULL, type, context, error);
}

/*
 * mono_class_inflate_generic_type_no_copy:
 *
 *   Same as inflate_generic_type_with_mempool, but return TYPE if no inflation
 * was done.
 */
static MonoType*
mono_class_inflate_generic_type_no_copy (MonoImage *image, MonoType *type, MonoGenericContext *context, MonoError *error)
{
	MonoType *inflated = NULL; 

	error_init (error);
	if (context) {
		inflated = inflate_generic_type (image, type, context, error);
		return_val_if_nok (error, NULL);
	}

	if (!inflated)
		return type;

	UnlockedIncrement (&mono_stats.inflated_type_count);
	return inflated;
}

/*
 * mono_class_inflate_generic_class:
 *
 *   Inflate the class @gklass with @context. Set @error on failure.
 */
MonoClass*
mono_class_inflate_generic_class_checked (MonoClass *gklass, MonoGenericContext *context, MonoError *error)
{
	MonoClass *res;
	MonoType *inflated;

	inflated = mono_class_inflate_generic_type_checked (&gklass->byval_arg, context, error);
	return_val_if_nok (error, NULL);

	res = mono_class_from_mono_type (inflated);
	mono_metadata_free_type (inflated);

	return res;
}

static MonoGenericContext
inflate_generic_context (MonoGenericContext *context, MonoGenericContext *inflate_with, MonoError *error)
{
	MonoGenericInst *class_inst = NULL;
	MonoGenericInst *method_inst = NULL;
	MonoGenericContext res = { NULL, NULL };

	error_init (error);

	if (context->class_inst) {
		class_inst = mono_metadata_inflate_generic_inst (context->class_inst, inflate_with, error);
		if (!mono_error_ok (error))
			goto fail;
	}

	if (context->method_inst) {
		method_inst = mono_metadata_inflate_generic_inst (context->method_inst, inflate_with, error);
		if (!mono_error_ok (error))
			goto fail;
	}

	res.class_inst = class_inst;
	res.method_inst = method_inst;
fail:
	return res;
}

/**
 * mono_class_inflate_generic_method:
 * \param method a generic method
 * \param context a generics context
 *
 * Instantiate the generic method \p method using the generics context \p context.
 *
 * \returns The new instantiated method
 */
MonoMethod *
mono_class_inflate_generic_method (MonoMethod *method, MonoGenericContext *context)
{
	ERROR_DECL (error);
	error_init (error);
	MonoMethod *res = mono_class_inflate_generic_method_full_checked (method, NULL, context, error);
	mono_error_assert_msg_ok (error, "Could not inflate generic method");
	return res;
}

MonoMethod *
mono_class_inflate_generic_method_checked (MonoMethod *method, MonoGenericContext *context, MonoError *error)
{
	return mono_class_inflate_generic_method_full_checked (method, NULL, context, error);
}

/**
 * mono_class_inflate_generic_method_full_checked:
 * Instantiate method \p method with the generic context \p context.
 * On failure returns NULL and sets \p error.
 *
 * BEWARE: All non-trivial fields are invalid, including klass, signature, and header.
 *         Use mono_method_signature() and mono_method_get_header() to get the correct values.
 */
MonoMethod*
mono_class_inflate_generic_method_full_checked (MonoMethod *method, MonoClass *klass_hint, MonoGenericContext *context, MonoError *error)
{
	MonoMethod *result;
	MonoMethodInflated *iresult, *cached;
	MonoMethodSignature *sig;
	MonoGenericContext tmp_context;

	error_init (error);

	/* The `method' has already been instantiated before => we need to peel out the instantiation and create a new context */
	while (method->is_inflated) {
		MonoGenericContext *method_context = mono_method_get_context (method);
		MonoMethodInflated *imethod = (MonoMethodInflated *) method;

		tmp_context = inflate_generic_context (method_context, context, error);
		return_val_if_nok (error, NULL);

		context = &tmp_context;

		if (mono_metadata_generic_context_equal (method_context, context))
			return method;

		method = imethod->declaring;
	}

	/*
	 * A method only needs to be inflated if the context has argument for which it is
	 * parametric. Eg:
	 * 
	 * class Foo<T> { void Bar(); } - doesn't need to be inflated if only mvars' are supplied
	 * class Foo { void Bar<T> (); } - doesn't need to be if only vars' are supplied
	 * 
	 */
	if (!((method->is_generic && context->method_inst) || 
		(mono_class_is_gtd (method->klass) && context->class_inst)))
		return method;

	iresult = g_new0 (MonoMethodInflated, 1);
	iresult->context = *context;
	iresult->declaring = method;

	if (!context->method_inst && method->is_generic)
		iresult->context.method_inst = mono_method_get_generic_container (method)->context.method_inst;

	if (!context->class_inst) {
		g_assert (!mono_class_is_ginst (iresult->declaring->klass));
		if (mono_class_is_gtd (iresult->declaring->klass))
			iresult->context.class_inst = mono_class_get_generic_container (iresult->declaring->klass)->context.class_inst;
	}
	/* This can happen with some callers like mono_object_get_virtual_method () */
	if (!mono_class_is_gtd (iresult->declaring->klass) && !mono_class_is_ginst (iresult->declaring->klass))
		iresult->context.class_inst = NULL;

	MonoImageSet *set = mono_metadata_get_image_set_for_method (iresult);

	// check cache
	mono_image_set_lock (set);
	cached = (MonoMethodInflated *)g_hash_table_lookup (set->gmethod_cache, iresult);
	mono_image_set_unlock (set);

	if (cached) {
		g_free (iresult);
		return (MonoMethod*)cached;
	}

	UnlockedIncrement (&mono_stats.inflated_method_count);

	UnlockedAdd (&inflated_methods_size,  sizeof (MonoMethodInflated));

	sig = mono_method_signature (method);
	if (!sig) {
		char *name = mono_type_get_full_name (method->klass);
		mono_error_set_bad_image (error, method->klass->image, "Could not resolve signature of method %s:%s", name, method->name);
		g_free (name);
		goto fail;
	}

	if (sig->pinvoke) {
		memcpy (&iresult->method.pinvoke, method, sizeof (MonoMethodPInvoke));
	} else {
		memcpy (&iresult->method.method, method, sizeof (MonoMethod));
	}

	result = (MonoMethod *) iresult;
	result->is_inflated = TRUE;
	result->is_generic = FALSE;
	result->sre_method = FALSE;
	result->signature = NULL;

	if (method->wrapper_type) {
		MonoMethodWrapper *mw = (MonoMethodWrapper*)method;
		MonoMethodWrapper *resw = (MonoMethodWrapper*)result;
		int len = GPOINTER_TO_INT (((void**)mw->method_data) [0]);

		resw->method_data = (void **)g_malloc (sizeof (gpointer) * (len + 1));
		memcpy (resw->method_data, mw->method_data, sizeof (gpointer) * (len + 1));
	}

	if (iresult->context.method_inst) {
		/* Set the generic_container of the result to the generic_container of method */
		MonoGenericContainer *generic_container = mono_method_get_generic_container (method);

		if (generic_container && iresult->context.method_inst == generic_container->context.method_inst) {
			result->is_generic = 1;
			mono_method_set_generic_container (result, generic_container);
		}
	}

	if (klass_hint) {
		MonoGenericClass *gklass_hint = mono_class_try_get_generic_class (klass_hint);
		if (gklass_hint && (gklass_hint->container_class != method->klass || gklass_hint->context.class_inst != context->class_inst))
			klass_hint = NULL;
	}

	if (mono_class_is_gtd (method->klass))
		result->klass = klass_hint;

	if (!result->klass) {
		MonoType *inflated = inflate_generic_type (NULL, &method->klass->byval_arg, context, error);
		if (!mono_error_ok (error)) 
			goto fail;

		result->klass = inflated ? mono_class_from_mono_type (inflated) : method->klass;
		if (inflated)
			mono_metadata_free_type (inflated);
	}

	/*
	 * FIXME: This should hold, but it doesn't:
	 *
	 * if (result->is_inflated && mono_method_get_context (result)->method_inst &&
	 *		mono_method_get_context (result)->method_inst == mono_method_get_generic_container (((MonoMethodInflated*)result)->declaring)->context.method_inst) {
	 * 	g_assert (result->is_generic);
	 * }
	 *
	 * Fixing this here causes other things to break, hence a very
	 * ugly hack in mini-trampolines.c - see
	 * is_generic_method_definition().
	 */

	// check cache
	mono_image_set_lock (set);
	cached = (MonoMethodInflated *)g_hash_table_lookup (set->gmethod_cache, iresult);
	if (!cached) {
		g_hash_table_insert (set->gmethod_cache, iresult, iresult);
		iresult->owner = set;
		cached = iresult;
	}
	mono_image_set_unlock (set);

	return (MonoMethod*)cached;

fail:
	g_free (iresult);
	return NULL;
}

/**
 * mono_get_inflated_method:
 *
 * Obsolete.  We keep it around since it's mentioned in the public API.
 */
MonoMethod*
mono_get_inflated_method (MonoMethod *method)
{
	return method;
}

/*
 * mono_method_get_context_general:
 * @method: a method
 * @uninflated: handle uninflated methods?
 *
 * Returns the generic context of a method or NULL if it doesn't have
 * one.  For an inflated method that's the context stored in the
 * method.  Otherwise it's in the method's generic container or in the
 * generic container of the method's class.
 */
MonoGenericContext*
mono_method_get_context_general (MonoMethod *method, gboolean uninflated)
{
	if (method->is_inflated) {
		MonoMethodInflated *imethod = (MonoMethodInflated *) method;
		return &imethod->context;
	}
	if (!uninflated)
		return NULL;
	if (method->is_generic)
		return &(mono_method_get_generic_container (method)->context);
	if (mono_class_is_gtd (method->klass))
		return &mono_class_get_generic_container (method->klass)->context;
	return NULL;
}

/*
 * mono_method_get_context:
 * @method: a method
 *
 * Returns the generic context for method if it's inflated, otherwise
 * NULL.
 */
MonoGenericContext*
mono_method_get_context (MonoMethod *method)
{
	return mono_method_get_context_general (method, FALSE);
}

/*
 * mono_method_get_generic_container:
 *
 *   Returns the generic container of METHOD, which should be a generic method definition.
 * Returns NULL if METHOD is not a generic method definition.
 * LOCKING: Acquires the loader lock.
 */
MonoGenericContainer*
mono_method_get_generic_container (MonoMethod *method)
{
	MonoGenericContainer *container;

	if (!method->is_generic)
		return NULL;

	container = (MonoGenericContainer *)mono_image_property_lookup (method->klass->image, method, MONO_METHOD_PROP_GENERIC_CONTAINER);
	g_assert (container);

	return container;
}

/*
 * mono_method_set_generic_container:
 *
 *   Sets the generic container of METHOD to CONTAINER.
 * LOCKING: Acquires the image lock.
 */
void
mono_method_set_generic_container (MonoMethod *method, MonoGenericContainer* container)
{
	g_assert (method->is_generic);

	mono_image_property_insert (method->klass->image, method, MONO_METHOD_PROP_GENERIC_CONTAINER, container);
}

/** 
 * mono_class_find_enum_basetype:
 * \param class The enum class
 *
 *   Determine the basetype of an enum by iterating through its fields. We do this
 * in a separate function since it is cheaper than calling mono_class_setup_fields.
 */
MonoType*
mono_class_find_enum_basetype (MonoClass *klass, MonoError *error)
{
	MonoGenericContainer *container = NULL;
	MonoImage *m = klass->image;
	const int top = mono_class_get_field_count (klass);
	int i, first_field_idx;

	g_assert (klass->enumtype);

	error_init (error);

	container = mono_class_try_get_generic_container (klass);
	if (mono_class_is_ginst (klass)) {
		MonoClass *gklass = mono_class_get_generic_class (klass)->container_class;

		container = mono_class_get_generic_container (gklass);
		g_assert (container);
	}

	/*
	 * Fetch all the field information.
	 */
	first_field_idx = mono_class_get_first_field_idx (klass);
	for (i = 0; i < top; i++){
		const char *sig;
		guint32 cols [MONO_FIELD_SIZE];
		int idx = first_field_idx + i;
		MonoType *ftype;

		/* first_field_idx and idx points into the fieldptr table */
		mono_metadata_decode_table_row (m, MONO_TABLE_FIELD, idx, cols, MONO_FIELD_SIZE);

		if (cols [MONO_FIELD_FLAGS] & FIELD_ATTRIBUTE_STATIC) //no need to decode static fields
			continue;

		if (!mono_verifier_verify_field_signature (klass->image, cols [MONO_FIELD_SIGNATURE], NULL)) {
			mono_error_set_bad_image (error, klass->image, "Invalid field signature %x", cols [MONO_FIELD_SIGNATURE]);
			goto fail;
		}

		sig = mono_metadata_blob_heap (m, cols [MONO_FIELD_SIGNATURE]);
		mono_metadata_decode_value (sig, &sig);
		/* FIELD signature == 0x06 */
		if (*sig != 0x06) {
			mono_error_set_bad_image (error, klass->image, "Invalid field signature %x, expected 0x6 but got %x", cols [MONO_FIELD_SIGNATURE], *sig);
			goto fail;
		}

		ftype = mono_metadata_parse_type_checked (m, container, cols [MONO_FIELD_FLAGS], FALSE, sig + 1, &sig, error);
		if (!ftype)
			goto fail;

		if (mono_class_is_ginst (klass)) {
			//FIXME do we leak here?
			ftype = mono_class_inflate_generic_type_checked (ftype, mono_class_get_context (klass), error);
			if (!mono_error_ok (error))
				goto fail;
			ftype->attrs = cols [MONO_FIELD_FLAGS];
		}

		return ftype;
	}
	mono_error_set_type_load_class (error, klass, "Could not find base type");

fail:
	return NULL;
}

/*
 * Checks for MonoClass::has_failure without resolving all MonoType's into MonoClass'es
 */
gboolean
mono_type_has_exceptions (MonoType *type)
{
	switch (type->type) {
	case MONO_TYPE_CLASS:
	case MONO_TYPE_VALUETYPE:
	case MONO_TYPE_SZARRAY:
		return mono_class_has_failure (type->data.klass);
	case MONO_TYPE_ARRAY:
		return mono_class_has_failure (type->data.array->eklass);
	case MONO_TYPE_GENERICINST:
		return mono_class_has_failure (mono_class_create_generic_inst (type->data.generic_class));
	default:
		return FALSE;
	}
}

void
mono_error_set_for_class_failure (MonoError *oerror, const MonoClass *klass)
{
	g_assert (mono_class_has_failure (klass));
	MonoErrorBoxed *box = mono_class_get_exception_data ((MonoClass*)klass);
	mono_error_set_from_boxed (oerror, box);
}

/*
 * mono_class_alloc:
 *
 *   Allocate memory for some data belonging to CLASS, either from its image's mempool,
 * or from the heap.
 */
gpointer
mono_class_alloc (MonoClass *klass, int size)
{
	MonoGenericClass *gklass = mono_class_try_get_generic_class (klass);
	if (gklass)
		return mono_image_set_alloc (gklass->owner, size);
	else
		return mono_image_alloc (klass->image, size);
}

gpointer
mono_class_alloc0 (MonoClass *klass, int size)
{
	gpointer res;

	res = mono_class_alloc (klass, size);
	memset (res, 0, size);
	return res;
}

#define mono_class_new0(klass,struct_type, n_structs)		\
    ((struct_type *) mono_class_alloc0 ((klass), ((gsize) sizeof (struct_type)) * ((gsize) (n_structs))))

/**
 * mono_class_set_failure_causedby_class:
 * \param klass the class that is failing
 * \param caused_by the class that caused the failure
 * \param msg Why \p klass is failing.
 * 
 * If \p caused_by has a failure, sets a TypeLoadException failure on
 * \p klass with message "\p msg, due to: {\p caused_by message}".
 *
 * \returns TRUE if a failiure was set, or FALSE if \p caused_by doesn't have a failure.
 */
gboolean
mono_class_set_type_load_failure_causedby_class (MonoClass *klass, const MonoClass *caused_by, const gchar* msg)
{
	if (mono_class_has_failure (caused_by)) {
		ERROR_DECL_VALUE (cause_error);
		error_init (&cause_error);
		mono_error_set_for_class_failure (&cause_error, caused_by);
		mono_class_set_type_load_failure (klass, "%s, due to: %s", msg, mono_error_get_message (&cause_error));
		mono_error_cleanup (&cause_error);
		return TRUE;
	} else {
		return FALSE;
	}
}


/*
 * mono_type_get_basic_type_from_generic:
 * @type: a type
 *
 * Returns a closed type corresponding to the possibly open type
 * passed to it.
 */
MonoType*
mono_type_get_basic_type_from_generic (MonoType *type)
{
	/* When we do generic sharing we let type variables stand for reference/primitive types. */
	if (!type->byref && (type->type == MONO_TYPE_VAR || type->type == MONO_TYPE_MVAR) &&
		(!type->data.generic_param->gshared_constraint || type->data.generic_param->gshared_constraint->type == MONO_TYPE_OBJECT))
		return &mono_defaults.object_class->byval_arg;
	return type;
}

/*
 * mono_class_get_method_by_index:
 *
 *   Returns klass->methods [index], initializing klass->methods if neccesary.
 *
 * LOCKING: Acquires the loader lock.
 */
MonoMethod*
mono_class_get_method_by_index (MonoClass *klass, int index)
{
	ERROR_DECL (error);

	MonoGenericClass *gklass = mono_class_try_get_generic_class (klass);
	/* Avoid calling setup_methods () if possible */
	if (gklass && !klass->methods) {
		MonoMethod *m;

		m = mono_class_inflate_generic_method_full_checked (
				gklass->container_class->methods [index], klass, mono_class_get_context (klass), error);
		g_assert (mono_error_ok (error)); /* FIXME don't swallow the error */
		/*
		 * If setup_methods () is called later for this class, no duplicates are created,
		 * since inflate_generic_method guarantees that only one instance of a method
		 * is created for each context.
		 */
		/*
		mono_class_setup_methods (klass);
		g_assert (m == klass->methods [index]);
		*/
		return m;
	} else {
		mono_class_setup_methods (klass);
		if (mono_class_has_failure (klass)) /*FIXME do proper error handling*/
			return NULL;
		g_assert (index >= 0 && index < mono_class_get_method_count (klass));
		return klass->methods [index];
	}
}	

/**
 * mono_class_get_inflated_method:
 * \param klass an inflated class
 * \param method a method of \p klass's generic definition
 * \param error set on error
 *
 * Given an inflated class \p klass and a method \p method which should be a
 * method of \p klass's generic definition, return the inflated method
 * corresponding to \p method.
 *
 * On failure sets \p error and returns NULL.
 */
MonoMethod*
mono_class_get_inflated_method (MonoClass *klass, MonoMethod *method, MonoError *error)
{
	MonoClass *gklass = mono_class_get_generic_class (klass)->container_class;
	int i, mcount;

	g_assert (method->klass == gklass);

	mono_class_setup_methods (gklass);
	if (mono_class_has_failure (gklass)) {
		mono_error_set_for_class_failure (error, gklass);
		return NULL;
	}

	mcount = mono_class_get_method_count (gklass);
	for (i = 0; i < mcount; ++i) {
		if (gklass->methods [i] == method) {
			MonoMethod *inflated_method = NULL;
			if (klass->methods) {
				inflated_method = klass->methods[i];
			} else {
				inflated_method = mono_class_inflate_generic_method_full_checked (gklass->methods [i], klass, mono_class_get_context (klass), error);
				return_val_if_nok (error, NULL);
			}
			g_assert (inflated_method);
			return inflated_method;
		}
	}

	g_assert_not_reached ();
}	

/*
 * mono_class_get_vtable_entry:
 *
 *   Returns klass->vtable [offset], computing it if neccesary. Returns NULL on failure.
 * LOCKING: Acquires the loader lock.
 */
MonoMethod*
mono_class_get_vtable_entry (MonoClass *klass, int offset)
{
	MonoMethod *m;

	if (klass->rank == 1) {
		/* 
		 * szarrays do not overwrite any methods of Array, so we can avoid
		 * initializing their vtables in some cases.
		 */
		mono_class_setup_vtable (klass->parent);
		if (offset < klass->parent->vtable_size)
			return klass->parent->vtable [offset];
	}

	if (mono_class_is_ginst (klass)) {
		ERROR_DECL (error);
		MonoClass *gklass = mono_class_get_generic_class (klass)->container_class;
		mono_class_setup_vtable (gklass);
		m = gklass->vtable [offset];

		m = mono_class_inflate_generic_method_full_checked (m, klass, mono_class_get_context (klass), error);
		g_assert (mono_error_ok (error)); /* FIXME don't swallow this error */
	} else {
		mono_class_setup_vtable (klass);
		if (mono_class_has_failure (klass))
			return NULL;
		m = klass->vtable [offset];
	}

	return m;
}

/*
 * mono_class_get_vtable_size:
 *
 *   Return the vtable size for KLASS.
 */
int
mono_class_get_vtable_size (MonoClass *klass)
{
	mono_class_setup_vtable (klass);

	return klass->vtable_size;
}

static void
collect_implemented_interfaces_aux (MonoClass *klass, GPtrArray **res, GHashTable **ifaces, MonoError *error)
{
	int i;
	MonoClass *ic;

	mono_class_setup_interfaces (klass, error);
	return_if_nok (error);

	for (i = 0; i < klass->interface_count; i++) {
		ic = klass->interfaces [i];

		if (*res == NULL)
			*res = g_ptr_array_new ();
		if (*ifaces == NULL)
			*ifaces = g_hash_table_new (NULL, NULL);
		if (g_hash_table_lookup (*ifaces, ic))
			continue;
		/* A gparam is not an implemented interface for the purposes of
		 * mono_class_get_implemented_interfaces */
		if (mono_class_is_gparam (ic))
			continue;
		g_ptr_array_add (*res, ic);
		g_hash_table_insert (*ifaces, ic, ic);
		mono_class_init (ic);
		if (mono_class_has_failure (ic)) {
			mono_error_set_type_load_class (error, ic, "Error Loading class");
			return;
		}

		collect_implemented_interfaces_aux (ic, res, ifaces, error);
		return_if_nok (error);
	}
}

GPtrArray*
mono_class_get_implemented_interfaces (MonoClass *klass, MonoError *error)
{
	GPtrArray *res = NULL;
	GHashTable *ifaces = NULL;

	collect_implemented_interfaces_aux (klass, &res, &ifaces, error);
	if (ifaces)
		g_hash_table_destroy (ifaces);
	if (!mono_error_ok (error)) {
		if (res)
			g_ptr_array_free (res, TRUE);
		return NULL;
	}
	return res;
}

static int
compare_interface_ids (const void *p_key, const void *p_element)
{
	const MonoClass *key = (const MonoClass *)p_key;
	const MonoClass *element = *(const MonoClass **)p_element;
	
	return (key->interface_id - element->interface_id);
}

/*FIXME verify all callers if they should switch to mono_class_interface_offset_with_variance*/
int
mono_class_interface_offset (MonoClass *klass, MonoClass *itf)
{
	MonoClass **result = (MonoClass **)mono_binary_search (
			itf,
			klass->interfaces_packed,
			klass->interface_offsets_count,
			sizeof (MonoClass *),
			compare_interface_ids);
	if (result) {
		return klass->interface_offsets_packed [result - (klass->interfaces_packed)];
	} else {
		return -1;
	}
}

/**
 * mono_class_interface_offset_with_variance:
 * 
 * Return the interface offset of \p itf in \p klass. Sets \p non_exact_match to TRUE if the match required variance check
 * If \p itf is an interface with generic variant arguments, try to find the compatible one.
 *
 * Note that this function is responsible for resolving ambiguities. Right now we use whatever ordering interfaces_packed gives us.
 *
 * FIXME figure out MS disambiguation rules and fix this function.
 */
int
mono_class_interface_offset_with_variance (MonoClass *klass, MonoClass *itf, gboolean *non_exact_match)
{
	int i = mono_class_interface_offset (klass, itf);
	*non_exact_match = FALSE;
	if (i >= 0)
		return i;

	if (itf->is_array_special_interface && klass->rank < 2) {
		MonoClass *gtd = mono_class_get_generic_type_definition (itf);
		int found = -1;

		for (i = 0; i < klass->interface_offsets_count; i++) {
			if (mono_class_is_variant_compatible (itf, klass->interfaces_packed [i], FALSE)) {
				found = i;
				*non_exact_match = TRUE;
				break;
			}

		}
		if (found != -1)
			return klass->interface_offsets_packed [found];

		for (i = 0; i < klass->interface_offsets_count; i++) {
			// printf ("\t%s\n", mono_type_get_full_name (klass->interfaces_packed [i]));
			if (mono_class_get_generic_type_definition (klass->interfaces_packed [i]) == gtd) {
				found = i;
				*non_exact_match = TRUE;
				break;
			}
		}

		if (found == -1)
			return -1;

		return klass->interface_offsets_packed [found];
	}

	if (!mono_class_has_variant_generic_params (itf))
		return -1;

	for (i = 0; i < klass->interface_offsets_count; i++) {
		if (mono_class_is_variant_compatible (itf, klass->interfaces_packed [i], FALSE)) {
			*non_exact_match = TRUE;
			return klass->interface_offsets_packed [i];
		}
	}

	return -1;
}


/*
 * mono_method_get_vtable_slot:
 *
 *   Returns method->slot, computing it if neccesary. Return -1 on failure.
 * LOCKING: Acquires the loader lock.
 *
 * FIXME Use proper MonoError machinery here.
 */
int
mono_method_get_vtable_slot (MonoMethod *method)
{
	if (method->slot == -1) {
		mono_class_setup_vtable (method->klass);
		if (mono_class_has_failure (method->klass))
			return -1;
		if (method->slot == -1) {
			MonoClass *gklass;
			int i, mcount;

			if (!mono_class_is_ginst (method->klass)) {
				g_assert (method->is_inflated);
				return mono_method_get_vtable_slot (((MonoMethodInflated*)method)->declaring);
			}

			/* This can happen for abstract methods of generic instances due to the shortcut code in mono_class_setup_vtable_general (). */
			g_assert (mono_class_is_ginst (method->klass));
			gklass = mono_class_get_generic_class (method->klass)->container_class;
			mono_class_setup_methods (method->klass);
			g_assert (method->klass->methods);
			mcount = mono_class_get_method_count (method->klass);
			for (i = 0; i < mcount; ++i) {
				if (method->klass->methods [i] == method)
					break;
			}
			g_assert (i < mcount);
			g_assert (gklass->methods);
			method->slot = gklass->methods [i]->slot;
		}
		g_assert (method->slot != -1);
	}
	return method->slot;
}

/**
 * mono_method_get_vtable_index:
 * \param method a method
 *
 * Returns the index into the runtime vtable to access the method or,
 * in the case of a virtual generic method, the virtual generic method
 * thunk. Returns -1 on failure.
 *
 * FIXME Use proper MonoError machinery here.
 */
int
mono_method_get_vtable_index (MonoMethod *method)
{
	if (method->is_inflated && (method->flags & METHOD_ATTRIBUTE_VIRTUAL)) {
		MonoMethodInflated *imethod = (MonoMethodInflated*)method;
		if (imethod->declaring->is_generic)
			return mono_method_get_vtable_slot (imethod->declaring);
	}
	return mono_method_get_vtable_slot (method);
}

/*
 * mono_class_has_finalizer:
 *
 *   Return whenever KLASS has a finalizer, initializing klass->has_finalizer in the
 * process.
 */
gboolean
mono_class_has_finalizer (MonoClass *klass)
{
	gboolean has_finalize = FALSE;

	if (klass->has_finalize_inited)
		return klass->has_finalize;

	/* Interfaces and valuetypes are not supposed to have finalizers */
	if (!(MONO_CLASS_IS_INTERFACE (klass) || klass->valuetype)) {
		MonoMethod *cmethod = NULL;

		if (klass->rank == 1 && klass->byval_arg.type == MONO_TYPE_SZARRAY) {
		} else if (mono_class_is_ginst (klass)) {
			MonoClass *gklass = mono_class_get_generic_class (klass)->container_class;

			has_finalize = mono_class_has_finalizer (gklass);
		} else if (klass->parent && klass->parent->has_finalize) {
			has_finalize = TRUE;
		} else {
			if (klass->parent) {
				/*
				 * Can't search in metadata for a method named Finalize, because that
				 * ignores overrides.
				 */
				mono_class_setup_vtable (klass);
				if (mono_class_has_failure (klass))
					cmethod = NULL;
				else
					cmethod = klass->vtable [mono_class_get_object_finalize_slot ()];
			}

			if (cmethod) {
				g_assert (klass->vtable_size > mono_class_get_object_finalize_slot ());

				if (klass->parent) {
					if (cmethod->is_inflated)
						cmethod = ((MonoMethodInflated*)cmethod)->declaring;
					if (cmethod != mono_class_get_default_finalize_method ())
						has_finalize = TRUE;
				}
			}
		}
	}

	mono_loader_lock ();
	if (!klass->has_finalize_inited) {
		klass->has_finalize = has_finalize ? 1 : 0;

		mono_memory_barrier ();
		klass->has_finalize_inited = TRUE;
	}
	mono_loader_unlock ();

	return klass->has_finalize;
}

gboolean
mono_is_corlib_image (MonoImage *image)
{
	return image == mono_defaults.corlib;
}

/*
 * mono_class_setup_supertypes:
 * @class: a class
 *
 * Build the data structure needed to make fast type checks work.
 * This currently sets two fields in @class:
 *  - idepth: distance between @class and System.Object in the type
 *    hierarchy + 1
 *  - supertypes: array of classes: each element has a class in the hierarchy
 *    starting from @class up to System.Object
 * 
 * LOCKING: Acquires the loader lock.
 */
void
mono_class_setup_supertypes (MonoClass *klass)
{
	int ms, idepth;
	MonoClass **supertypes;

	mono_atomic_load_acquire (supertypes, MonoClass **, &klass->supertypes);
	if (supertypes)
		return;

	if (klass->parent && !klass->parent->supertypes)
		mono_class_setup_supertypes (klass->parent);
	if (klass->parent)
		idepth = klass->parent->idepth + 1;
	else
		idepth = 1;

	ms = MAX (MONO_DEFAULT_SUPERTABLE_SIZE, idepth);
	supertypes = (MonoClass **)mono_class_alloc0 (klass, sizeof (MonoClass *) * ms);

	if (klass->parent) {
		CHECKED_METADATA_WRITE_PTR ( supertypes [idepth - 1] , klass );

		int supertype_idx;
		for (supertype_idx = 0; supertype_idx < klass->parent->idepth; supertype_idx++)
			CHECKED_METADATA_WRITE_PTR ( supertypes [supertype_idx] , klass->parent->supertypes [supertype_idx] );
	} else {
		CHECKED_METADATA_WRITE_PTR ( supertypes [0] , klass );
	}

	mono_memory_barrier ();

	mono_loader_lock ();
	klass->idepth = idepth;
	/* Needed so idepth is visible before supertypes is set */
	mono_memory_barrier ();
	klass->supertypes = supertypes;
	mono_loader_unlock ();
}

/** Is klass a Nullable<T> ginst? */
gboolean
mono_class_is_nullable (MonoClass *klass)
{
	MonoGenericClass *gklass = mono_class_try_get_generic_class (klass);
	return gklass && gklass->container_class == mono_defaults.generic_nullable_class;
}


/** if klass is T? return T */
MonoClass*
mono_class_get_nullable_param (MonoClass *klass)
{
       g_assert (mono_class_is_nullable (klass));
       return mono_class_from_mono_type (mono_class_get_generic_class (klass)->context.class_inst->type_argv [0]);
}

gboolean
mono_type_is_primitive (MonoType *type)
{
	return (type->type >= MONO_TYPE_BOOLEAN && type->type <= MONO_TYPE_R8) ||
			type-> type == MONO_TYPE_I || type->type == MONO_TYPE_U;
}

static MonoImage *
get_image_for_container (MonoGenericContainer *container)
{
	MonoImage *result;
	if (container->is_anonymous) {
		result = container->owner.image;
	} else {
		MonoClass *klass;
		if (container->is_method) {
			MonoMethod *method = container->owner.method;
			g_assert_checked (method);
			klass = method->klass;
		} else {
			klass = container->owner.klass;
		}
		g_assert_checked (klass);
		result = klass->image;
	}
	g_assert (result);
	return result;
}

MonoImage *
get_image_for_generic_param (MonoGenericParam *param)
{
	MonoGenericContainer *container = mono_generic_param_owner (param);
	g_assert_checked (container);
	return get_image_for_container (container);
}

// Make a string in the designated image consisting of a single integer.
#define INT_STRING_SIZE 16
char *
make_generic_name_string (MonoImage *image, int num)
{
	char *name = (char *)mono_image_alloc0 (image, INT_STRING_SIZE);
	g_snprintf (name, INT_STRING_SIZE, "%d", num);
	return name;
}

/**
 * mono_class_from_generic_parameter:
 * \param param Parameter to find/construct a class for.
 * \param arg2 Is ignored.
 * \param arg3 Is ignored.
 */
MonoClass *
mono_class_from_generic_parameter (MonoGenericParam *param, MonoImage *arg2 G_GNUC_UNUSED, gboolean arg3 G_GNUC_UNUSED)
{
	return mono_class_create_generic_parameter (param);
}

/**
 * mono_ptr_class_get:
 */
MonoClass *
mono_ptr_class_get (MonoType *type)
{
	return mono_class_create_ptr (type);
}

/**
 * mono_class_from_mono_type:
 * \param type describes the type to return
 * \returns a \c MonoClass for the specified \c MonoType, the value is never NULL.
 */
MonoClass *
mono_class_from_mono_type (MonoType *type)
{
	switch (type->type) {
	case MONO_TYPE_OBJECT:
		return type->data.klass? type->data.klass: mono_defaults.object_class;
	case MONO_TYPE_VOID:
		return type->data.klass? type->data.klass: mono_defaults.void_class;
	case MONO_TYPE_BOOLEAN:
		return type->data.klass? type->data.klass: mono_defaults.boolean_class;
	case MONO_TYPE_CHAR:
		return type->data.klass? type->data.klass: mono_defaults.char_class;
	case MONO_TYPE_I1:
		return type->data.klass? type->data.klass: mono_defaults.sbyte_class;
	case MONO_TYPE_U1:
		return type->data.klass? type->data.klass: mono_defaults.byte_class;
	case MONO_TYPE_I2:
		return type->data.klass? type->data.klass: mono_defaults.int16_class;
	case MONO_TYPE_U2:
		return type->data.klass? type->data.klass: mono_defaults.uint16_class;
	case MONO_TYPE_I4:
		return type->data.klass? type->data.klass: mono_defaults.int32_class;
	case MONO_TYPE_U4:
		return type->data.klass? type->data.klass: mono_defaults.uint32_class;
	case MONO_TYPE_I:
		return type->data.klass? type->data.klass: mono_defaults.int_class;
	case MONO_TYPE_U:
		return type->data.klass? type->data.klass: mono_defaults.uint_class;
	case MONO_TYPE_I8:
		return type->data.klass? type->data.klass: mono_defaults.int64_class;
	case MONO_TYPE_U8:
		return type->data.klass? type->data.klass: mono_defaults.uint64_class;
	case MONO_TYPE_R4:
		return type->data.klass? type->data.klass: mono_defaults.single_class;
	case MONO_TYPE_R8:
		return type->data.klass? type->data.klass: mono_defaults.double_class;
	case MONO_TYPE_STRING:
		return type->data.klass? type->data.klass: mono_defaults.string_class;
	case MONO_TYPE_TYPEDBYREF:
		return type->data.klass? type->data.klass: mono_defaults.typed_reference_class;
	case MONO_TYPE_ARRAY:
		return mono_class_create_bounded_array (type->data.array->eklass, type->data.array->rank, TRUE);
	case MONO_TYPE_PTR:
		return mono_class_create_ptr (type->data.type);
	case MONO_TYPE_FNPTR:
		return mono_class_create_fnptr (type->data.method);
	case MONO_TYPE_SZARRAY:
		return mono_class_create_array (type->data.klass, 1);
	case MONO_TYPE_CLASS:
	case MONO_TYPE_VALUETYPE:
		return type->data.klass;
	case MONO_TYPE_GENERICINST:
		return mono_class_create_generic_inst (type->data.generic_class);
	case MONO_TYPE_MVAR:
	case MONO_TYPE_VAR:
		return mono_class_create_generic_parameter (type->data.generic_param);
	default:
		g_warning ("mono_class_from_mono_type: implement me 0x%02x\n", type->type);
		g_assert_not_reached ();
	}

	// Yes, this returns NULL, even if it is documented as not doing so, but there
	// is no way for the code to make it this far, due to the assert above.
	return NULL;
}

/**
 * mono_type_retrieve_from_typespec
 * \param image context where the image is created
 * \param type_spec  typespec token
 * \param context the generic context used to evaluate generic instantiations in
 */
static MonoType *
mono_type_retrieve_from_typespec (MonoImage *image, guint32 type_spec, MonoGenericContext *context, gboolean *did_inflate, MonoError *error)
{
	MonoType *t = mono_type_create_from_typespec_checked (image, type_spec, error);

	*did_inflate = FALSE;

	if (!t)
		return NULL;

	if (context && (context->class_inst || context->method_inst)) {
		MonoType *inflated = inflate_generic_type (NULL, t, context, error);

		if (!mono_error_ok (error)) {
			return NULL;
		}

		if (inflated) {
			t = inflated;
			*did_inflate = TRUE;
		}
	}
	return t;
}

/**
 * mono_class_create_from_typespec
 * \param image context where the image is created
 * \param type_spec typespec token
 * \param context the generic context used to evaluate generic instantiations in
 */
static MonoClass *
mono_class_create_from_typespec (MonoImage *image, guint32 type_spec, MonoGenericContext *context, MonoError *error)
{
	MonoClass *ret;
	gboolean inflated = FALSE;
	MonoType *t = mono_type_retrieve_from_typespec (image, type_spec, context, &inflated, error);
	return_val_if_nok (error, NULL);
	ret = mono_class_from_mono_type (t);
	if (inflated)
		mono_metadata_free_type (t);
	return ret;
}

/**
 * mono_bounded_array_class_get:
 * \param element_class element class 
 * \param rank the dimension of the array class
 * \param bounded whenever the array has non-zero bounds
 * \returns A class object describing the array with element type \p element_type and 
 * dimension \p rank.
 */
MonoClass *
mono_bounded_array_class_get (MonoClass *eclass, guint32 rank, gboolean bounded)
{
	return mono_class_create_bounded_array (eclass, rank, bounded);
}

/**
 * mono_array_class_get:
 * \param element_class element class 
 * \param rank the dimension of the array class
 * \returns A class object describing the array with element type \p element_type and 
 * dimension \p rank.
 */
MonoClass *
mono_array_class_get (MonoClass *eclass, guint32 rank)
{
	return mono_class_create_array (eclass, rank);
}

/**
 * mono_class_instance_size:
 * \param klass a class
 *
 * Use to get the size of a class in bytes.
 *
 * \returns The size of an object instance
 */
gint32
mono_class_instance_size (MonoClass *klass)
{	
	if (!klass->size_inited)
		mono_class_init (klass);

	return klass->instance_size;
}

/**
 * mono_class_min_align:
 * \param klass a class 
 *
 * Use to get the computed minimum alignment requirements for the specified class.
 *
 * Returns: minimum alignment requirements
 */
gint32
mono_class_min_align (MonoClass *klass)
{	
	if (!klass->size_inited)
		mono_class_init (klass);

	return klass->min_align;
}

/**
 * mono_class_value_size:
 * \param klass a class 
 *
 * This function is used for value types, and return the
 * space and the alignment to store that kind of value object.
 *
 * \returns the size of a value of kind \p klass
 */
gint32
mono_class_value_size (MonoClass *klass, guint32 *align)
{
	gint32 size;

	/* fixme: check disable, because we still have external revereces to
	 * mscorlib and Dummy Objects 
	 */
	/*g_assert (klass->valuetype);*/

	size = mono_class_instance_size (klass) - sizeof (MonoObject);

	if (align)
		*align = klass->min_align;

	return size;
}

/**
 * mono_class_data_size:
 * \param klass a class 
 * 
 * \returns The size of the static class data
 */
gint32
mono_class_data_size (MonoClass *klass)
{	
	if (!klass->inited)
		mono_class_init (klass);
	/* This can happen with dynamically created types */
	if (!klass->fields_inited)
		mono_class_setup_fields (klass);

	/* in arrays, sizes.class_size is unioned with element_size
	 * and arrays have no static fields
	 */
	if (klass->rank)
		return 0;
	return klass->sizes.class_size;
}

/*
 * Auxiliary routine to mono_class_get_field
 *
 * Takes a field index instead of a field token.
 */
static MonoClassField *
mono_class_get_field_idx (MonoClass *klass, int idx)
{
	mono_class_setup_fields (klass);
	if (mono_class_has_failure (klass))
		return NULL;

	while (klass) {
		int first_field_idx = mono_class_get_first_field_idx (klass);
		int fcount = mono_class_get_field_count (klass);
		if (klass->image->uncompressed_metadata) {
			/* 
			 * first_field_idx points to the FieldPtr table, while idx points into the
			 * Field table, so we have to do a search.
			 */
			/*FIXME this is broken for types with multiple fields with the same name.*/
			const char *name = mono_metadata_string_heap (klass->image, mono_metadata_decode_row_col (&klass->image->tables [MONO_TABLE_FIELD], idx, MONO_FIELD_NAME));
			int i;

			for (i = 0; i < fcount; ++i)
				if (mono_field_get_name (&klass->fields [i]) == name)
					return &klass->fields [i];
			g_assert_not_reached ();
		} else {			
			if (fcount) {
				if ((idx >= first_field_idx) && (idx < first_field_idx + fcount)){
					return &klass->fields [idx - first_field_idx];
				}
			}
		}
		klass = klass->parent;
	}
	return NULL;
}

/**
 * mono_class_get_field:
 * \param class the class to lookup the field.
 * \param field_token the field token
 *
 * \returns A \c MonoClassField representing the type and offset of
 * the field, or a NULL value if the field does not belong to this
 * class.
 */
MonoClassField *
mono_class_get_field (MonoClass *klass, guint32 field_token)
{
	int idx = mono_metadata_token_index (field_token);

	g_assert (mono_metadata_token_code (field_token) == MONO_TOKEN_FIELD_DEF);

	return mono_class_get_field_idx (klass, idx - 1);
}

/**
 * mono_class_get_field_from_name:
 * \param klass the class to lookup the field.
 * \param name the field name
 *
 * Search the class \p klass and its parents for a field with the name \p name.
 * 
 * \returns The \c MonoClassField pointer of the named field or NULL
 */
MonoClassField *
mono_class_get_field_from_name (MonoClass *klass, const char *name)
{
	return mono_class_get_field_from_name_full (klass, name, NULL);
}

/**
 * mono_class_get_field_from_name_full:
 * \param klass the class to lookup the field.
 * \param name the field name
 * \param type the type of the fields. This optional.
 *
 * Search the class \p klass and it's parents for a field with the name \p name and type \p type.
 *
 * If \p klass is an inflated generic type, the type comparison is done with the equivalent field
 * of its generic type definition.
 *
 * \returns The MonoClassField pointer of the named field or NULL
 */
MonoClassField *
mono_class_get_field_from_name_full (MonoClass *klass, const char *name, MonoType *type)
{
	int i;

	mono_class_setup_fields (klass);
	if (mono_class_has_failure (klass))
		return NULL;

	while (klass) {
		int fcount = mono_class_get_field_count (klass);
		for (i = 0; i < fcount; ++i) {
			MonoClassField *field = &klass->fields [i];

			if (strcmp (name, mono_field_get_name (field)) != 0)
				continue;

			if (type) {
				MonoType *field_type = mono_metadata_get_corresponding_field_from_generic_type_definition (field)->type;
				if (!mono_metadata_type_equal_full (type, field_type, TRUE))
					continue;
			}
			return field;
		}
		klass = klass->parent;
	}
	return NULL;
}

/**
 * mono_class_get_field_token:
 * \param field the field we need the token of
 *
 * Get the token of a field. Note that the tokesn is only valid for the image
 * the field was loaded from. Don't use this function for fields in dynamic types.
 * 
 * \returns The token representing the field in the image it was loaded from.
 */
guint32
mono_class_get_field_token (MonoClassField *field)
{
	MonoClass *klass = field->parent;
	int i;

	mono_class_setup_fields (klass);

	while (klass) {
		if (!klass->fields)
			return 0;
		int first_field_idx = mono_class_get_first_field_idx (klass);
		int fcount = mono_class_get_field_count (klass);
		for (i = 0; i < fcount; ++i) {
			if (&klass->fields [i] == field) {
				int idx = first_field_idx + i + 1;

				if (klass->image->uncompressed_metadata)
					idx = mono_metadata_translate_token_index (klass->image, MONO_TABLE_FIELD, idx);
				return mono_metadata_make_token (MONO_TABLE_FIELD, idx);
			}
		}
		klass = klass->parent;
	}

	g_assert_not_reached ();
	return 0;
}

static int
mono_field_get_index (MonoClassField *field)
{
	int index = field - field->parent->fields;
	g_assert (index >= 0 && index < mono_class_get_field_count (field->parent));

	return index;
}

/*
 * mono_class_get_field_default_value:
 *
 * Return the default value of the field as a pointer into the metadata blob.
 */
const char*
mono_class_get_field_default_value (MonoClassField *field, MonoTypeEnum *def_type)
{
	guint32 cindex;
	guint32 constant_cols [MONO_CONSTANT_SIZE];
	int field_index;
	MonoClass *klass = field->parent;
	MonoFieldDefaultValue *def_values;

	g_assert (field->type->attrs & FIELD_ATTRIBUTE_HAS_DEFAULT);

	def_values = mono_class_get_field_def_values (klass);
	if (!def_values) {
		def_values = (MonoFieldDefaultValue *)mono_class_alloc0 (klass, sizeof (MonoFieldDefaultValue) * mono_class_get_field_count (klass));

		mono_class_set_field_def_values (klass, def_values);
	}

	field_index = mono_field_get_index (field);
		
	if (!def_values [field_index].data) {
		cindex = mono_metadata_get_constant_index (field->parent->image, mono_class_get_field_token (field), 0);
		if (!cindex)
			return NULL;

		g_assert (!(field->type->attrs & FIELD_ATTRIBUTE_HAS_FIELD_RVA));

		mono_metadata_decode_row (&field->parent->image->tables [MONO_TABLE_CONSTANT], cindex - 1, constant_cols, MONO_CONSTANT_SIZE);
		def_values [field_index].def_type = (MonoTypeEnum)constant_cols [MONO_CONSTANT_TYPE];
		mono_memory_barrier ();
		def_values [field_index].data = (const char *)mono_metadata_blob_heap (field->parent->image, constant_cols [MONO_CONSTANT_VALUE]);
	}

	*def_type = def_values [field_index].def_type;
	return def_values [field_index].data;
}

static int
mono_property_get_index (MonoProperty *prop)
{
	MonoClassPropertyInfo *info = mono_class_get_property_info (prop->parent);
	int index = prop - info->properties;

	g_assert (index >= 0 && index < info->count);

	return index;
}

/*
 * mono_class_get_property_default_value:
 *
 * Return the default value of the field as a pointer into the metadata blob.
 */
const char*
mono_class_get_property_default_value (MonoProperty *property, MonoTypeEnum *def_type)
{
	guint32 cindex;
	guint32 constant_cols [MONO_CONSTANT_SIZE];
	MonoClass *klass = property->parent;

	g_assert (property->attrs & PROPERTY_ATTRIBUTE_HAS_DEFAULT);
	/*
	 * We don't cache here because it is not used by C# so it's quite rare, but
	 * we still do the lookup in klass->ext because that is where the data
	 * is stored for dynamic assemblies.
	 */

	if (image_is_dynamic (klass->image)) {
		MonoClassPropertyInfo *info = mono_class_get_property_info (klass);
		int prop_index = mono_property_get_index (property);
		if (info->def_values && info->def_values [prop_index].data) {
			*def_type = info->def_values [prop_index].def_type;
			return info->def_values [prop_index].data;
		}
		return NULL;
	}
	cindex = mono_metadata_get_constant_index (klass->image, mono_class_get_property_token (property), 0);
	if (!cindex)
		return NULL;

	mono_metadata_decode_row (&klass->image->tables [MONO_TABLE_CONSTANT], cindex - 1, constant_cols, MONO_CONSTANT_SIZE);
	*def_type = (MonoTypeEnum)constant_cols [MONO_CONSTANT_TYPE];
	return (const char *)mono_metadata_blob_heap (klass->image, constant_cols [MONO_CONSTANT_VALUE]);
}

/**
 * mono_class_get_event_token:
 */
guint32
mono_class_get_event_token (MonoEvent *event)
{
	MonoClass *klass = event->parent;
	int i;

	while (klass) {
		MonoClassEventInfo *info = mono_class_get_event_info (klass);
		if (info) {
			for (i = 0; i < info->count; ++i) {
				if (&info->events [i] == event)
					return mono_metadata_make_token (MONO_TABLE_EVENT, info->first + i + 1);
			}
		}
		klass = klass->parent;
	}

	g_assert_not_reached ();
	return 0;
}

/**
 * mono_class_get_property_from_name:
 * \param klass a class
 * \param name name of the property to lookup in the specified class
 *
 * Use this method to lookup a property in a class
 * \returns the \c MonoProperty with the given name, or NULL if the property
 * does not exist on the \p klass.
 */
MonoProperty*
mono_class_get_property_from_name (MonoClass *klass, const char *name)
{
	while (klass) {
		MonoProperty* p;
		gpointer iter = NULL;
		while ((p = mono_class_get_properties (klass, &iter))) {
			if (! strcmp (name, p->name))
				return p;
		}
		klass = klass->parent;
	}
	return NULL;
}

/**
 * mono_class_get_property_token:
 * \param prop MonoProperty to query
 *
 * \returns The ECMA token for the specified property.
 */
guint32
mono_class_get_property_token (MonoProperty *prop)
{
	MonoClass *klass = prop->parent;
	while (klass) {
		MonoProperty* p;
		int i = 0;
		gpointer iter = NULL;
		MonoClassPropertyInfo *info = mono_class_get_property_info (klass);
		while ((p = mono_class_get_properties (klass, &iter))) {
			if (&info->properties [i] == prop)
				return mono_metadata_make_token (MONO_TABLE_PROPERTY, info->first + i + 1);
			
			i ++;
		}
		klass = klass->parent;
	}

	g_assert_not_reached ();
	return 0;
}

/**
 * mono_class_name_from_token:
 */
char *
mono_class_name_from_token (MonoImage *image, guint32 type_token)
{
	const char *name, *nspace;
	if (image_is_dynamic (image))
		return g_strdup_printf ("DynamicType 0x%08x", type_token);
	
	switch (type_token & 0xff000000){
	case MONO_TOKEN_TYPE_DEF: {
		guint32 cols [MONO_TYPEDEF_SIZE];
		MonoTableInfo *tt = &image->tables [MONO_TABLE_TYPEDEF];
		guint tidx = mono_metadata_token_index (type_token);

		if (tidx > tt->rows)
			return g_strdup_printf ("Invalid type token 0x%08x", type_token);

		mono_metadata_decode_row (tt, tidx - 1, cols, MONO_TYPEDEF_SIZE);
		name = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAME]);
		nspace = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAMESPACE]);
		if (strlen (nspace) == 0)
			return g_strdup_printf ("%s", name);
		else
			return g_strdup_printf ("%s.%s", nspace, name);
	}

	case MONO_TOKEN_TYPE_REF: {
		ERROR_DECL (error);
		guint32 cols [MONO_TYPEREF_SIZE];
		MonoTableInfo  *t = &image->tables [MONO_TABLE_TYPEREF];
		guint tidx = mono_metadata_token_index (type_token);

		if (tidx > t->rows)
			return g_strdup_printf ("Invalid type token 0x%08x", type_token);

		if (!mono_verifier_verify_typeref_row (image, tidx - 1, error)) {
			char *msg = g_strdup_printf ("Invalid type token 0x%08x due to '%s'", type_token, mono_error_get_message (error));
			mono_error_cleanup (error);
			return msg;
		}

		mono_metadata_decode_row (t, tidx-1, cols, MONO_TYPEREF_SIZE);
		name = mono_metadata_string_heap (image, cols [MONO_TYPEREF_NAME]);
		nspace = mono_metadata_string_heap (image, cols [MONO_TYPEREF_NAMESPACE]);
		if (strlen (nspace) == 0)
			return g_strdup_printf ("%s", name);
		else
			return g_strdup_printf ("%s.%s", nspace, name);
	}
		
	case MONO_TOKEN_TYPE_SPEC:
		return g_strdup_printf ("Typespec 0x%08x", type_token);
	default:
		return g_strdup_printf ("Invalid type token 0x%08x", type_token);
	}
}

static char *
mono_assembly_name_from_token (MonoImage *image, guint32 type_token)
{
	if (image_is_dynamic (image))
		return g_strdup_printf ("DynamicAssembly %s", image->name);
	
	switch (type_token & 0xff000000){
	case MONO_TOKEN_TYPE_DEF:
		if (image->assembly)
			return mono_stringify_assembly_name (&image->assembly->aname);
		else if (image->assembly_name)
			return g_strdup (image->assembly_name);
		return g_strdup_printf ("%s", image->name ? image->name : "[Could not resolve assembly name");
	case MONO_TOKEN_TYPE_REF: {
		ERROR_DECL (error);
		MonoAssemblyName aname;
		guint32 cols [MONO_TYPEREF_SIZE];
		MonoTableInfo  *t = &image->tables [MONO_TABLE_TYPEREF];
		guint32 idx = mono_metadata_token_index (type_token);

		if (idx > t->rows)
			return g_strdup_printf ("Invalid type token 0x%08x", type_token);
	
		if (!mono_verifier_verify_typeref_row (image, idx - 1, error)) {
			char *msg = g_strdup_printf ("Invalid type token 0x%08x due to '%s'", type_token, mono_error_get_message (error));
			mono_error_cleanup (error);
			return msg;
		}
		mono_metadata_decode_row (t, idx-1, cols, MONO_TYPEREF_SIZE);

		idx = cols [MONO_TYPEREF_SCOPE] >> MONO_RESOLUTION_SCOPE_BITS;
		switch (cols [MONO_TYPEREF_SCOPE] & MONO_RESOLUTION_SCOPE_MASK) {
		case MONO_RESOLUTION_SCOPE_MODULE:
			/* FIXME: */
			return g_strdup ("");
		case MONO_RESOLUTION_SCOPE_MODULEREF:
			/* FIXME: */
			return g_strdup ("");
		case MONO_RESOLUTION_SCOPE_TYPEREF:
			/* FIXME: */
			return g_strdup ("");
		case MONO_RESOLUTION_SCOPE_ASSEMBLYREF:
			mono_assembly_get_assemblyref (image, idx - 1, &aname);
			return mono_stringify_assembly_name (&aname);
		default:
			g_assert_not_reached ();
		}
		break;
	}
	case MONO_TOKEN_TYPE_SPEC:
		/* FIXME: */
		return g_strdup ("");
	default:
		g_assert_not_reached ();
	}

	return NULL;
}

/**
 * mono_class_get_full:
 * \param image the image where the class resides
 * \param type_token the token for the class
 * \param context the generic context used to evaluate generic instantiations in
 * \deprecated Functions that expose \c MonoGenericContext are going away in mono 4.0
 * \returns The \c MonoClass that represents \p type_token in \p image
 */
MonoClass *
mono_class_get_full (MonoImage *image, guint32 type_token, MonoGenericContext *context)
{
	ERROR_DECL (error);
	MonoClass *klass;
	klass = mono_class_get_checked (image, type_token, error);

	if (klass && context && mono_metadata_token_table (type_token) == MONO_TABLE_TYPESPEC)
		klass = mono_class_inflate_generic_class_checked (klass, context, error);

	mono_error_assert_ok (error);
	return klass;
}


MonoClass *
mono_class_get_and_inflate_typespec_checked (MonoImage *image, guint32 type_token, MonoGenericContext *context, MonoError *error)
{
	MonoClass *klass;

	error_init (error);
	klass = mono_class_get_checked (image, type_token, error);

	if (klass && context && mono_metadata_token_table (type_token) == MONO_TABLE_TYPESPEC)
		klass = mono_class_inflate_generic_class_checked (klass, context, error);

	return klass;
}
/**
 * mono_class_get_checked:
 * \param image the image where the class resides
 * \param type_token the token for the class
 * \param error error object to return any error
 *
 * \returns The MonoClass that represents \p type_token in \p image, or NULL on error.
 */
MonoClass *
mono_class_get_checked (MonoImage *image, guint32 type_token, MonoError *error)
{
	MonoClass *klass = NULL;

	error_init (error);

	if (image_is_dynamic (image)) {
		int table = mono_metadata_token_table (type_token);

		if (table != MONO_TABLE_TYPEDEF && table != MONO_TABLE_TYPEREF && table != MONO_TABLE_TYPESPEC) {
			mono_error_set_bad_image (error, image,"Bad token table for dynamic image: %x", table);
			return NULL;
		}
		klass = (MonoClass *)mono_lookup_dynamic_token (image, type_token, NULL, error);
		goto done;
	}

	switch (type_token & 0xff000000){
	case MONO_TOKEN_TYPE_DEF:
		klass = mono_class_create_from_typedef (image, type_token, error);
		break;		
	case MONO_TOKEN_TYPE_REF:
		klass = mono_class_from_typeref_checked (image, type_token, error);
		break;
	case MONO_TOKEN_TYPE_SPEC:
		klass = mono_class_create_from_typespec (image, type_token, NULL, error);
		break;
	default:
		mono_error_set_bad_image (error, image, "Unknown type token %x", type_token & 0xff000000);
	}

done:
	/* Generic case, should be avoided for when a better error is possible. */
	if (!klass && mono_error_ok (error)) {
		char *name = mono_class_name_from_token (image, type_token);
		char *assembly = mono_assembly_name_from_token (image, type_token);
		mono_error_set_type_load_name (error, name, assembly, "Could not resolve type with token %08x (expected class '%s' in assembly '%s')", type_token, name, assembly);
	}

	return klass;
}


/**
 * mono_type_get_checked:
 * \param image the image where the type resides
 * \param type_token the token for the type
 * \param context the generic context used to evaluate generic instantiations in
 * \param error Error handling context
 *
 * This functions exists to fullfill the fact that sometimes it's desirable to have access to the 
 * 
 * \returns The MonoType that represents \p type_token in \p image
 */
MonoType *
mono_type_get_checked (MonoImage *image, guint32 type_token, MonoGenericContext *context, MonoError *error)
{
	MonoType *type = NULL;
	gboolean inflated = FALSE;

	error_init (error);

	//FIXME: this will not fix the very issue for which mono_type_get_full exists -but how to do it then?
	if (image_is_dynamic (image)) {
		MonoClass *klass = (MonoClass *)mono_lookup_dynamic_token (image, type_token, context, error);
		return_val_if_nok (error, NULL);
		return mono_class_get_type (klass);
	}

	if ((type_token & 0xff000000) != MONO_TOKEN_TYPE_SPEC) {
		MonoClass *klass = mono_class_get_checked (image, type_token, error);

		if (!klass) {
			return NULL;
		}

		g_assert (klass);
		return mono_class_get_type (klass);
	}

	type = mono_type_retrieve_from_typespec (image, type_token, context, &inflated, error);

	if (!type) {
		return NULL;
	}

	if (inflated) {
		MonoType *tmp = type;
		type = mono_class_get_type (mono_class_from_mono_type (type));
		/* FIXME: This is a workaround fo the fact that a typespec token sometimes reference to the generic type definition.
		 * A MonoClass::byval_arg of a generic type definion has type CLASS.
		 * Some parts of mono create a GENERICINST to reference a generic type definition and this generates confict with byval_arg.
		 *
		 * The long term solution is to chaise this places and make then set MonoType::type correctly.
		 * */
		if (type->type != tmp->type)
			type = tmp;
		else
			mono_metadata_free_type (tmp);
	}
	return type;
}

/**
 * mono_class_get:
 * \param image image where the class token will be looked up.
 * \param type_token a type token from the image
 * \returns the \c MonoClass with the given \p type_token on the \p image
 */
MonoClass *
mono_class_get (MonoImage *image, guint32 type_token)
{
	ERROR_DECL (error);
	error_init (error);
	MonoClass *result = mono_class_get_checked (image, type_token, error);
	mono_error_assert_ok (error);
	return result;
}

/**
 * mono_image_init_name_cache:
 *
 *  Initializes the class name cache stored in image->name_cache.
 *
 * LOCKING: Acquires the corresponding image lock.
 */
void
mono_image_init_name_cache (MonoImage *image)
{
	MonoTableInfo  *t = &image->tables [MONO_TABLE_TYPEDEF];
	guint32 cols [MONO_TYPEDEF_SIZE];
	const char *name;
	const char *nspace;
	guint32 i, visib, nspace_index;
	GHashTable *name_cache2, *nspace_table, *the_name_cache;

	if (image->name_cache)
		return;

	the_name_cache = g_hash_table_new (g_str_hash, g_str_equal);

	if (image_is_dynamic (image)) {
		mono_image_lock (image);
		if (image->name_cache) {
			/* Somebody initialized it before us */
			g_hash_table_destroy (the_name_cache);
		} else {
			mono_atomic_store_release (&image->name_cache, the_name_cache);
		}
		mono_image_unlock (image);
		return;
	}

	/* Temporary hash table to avoid lookups in the nspace_table */
	name_cache2 = g_hash_table_new (NULL, NULL);

	for (i = 1; i <= t->rows; ++i) {
		mono_metadata_decode_row (t, i - 1, cols, MONO_TYPEDEF_SIZE);
		visib = cols [MONO_TYPEDEF_FLAGS] & TYPE_ATTRIBUTE_VISIBILITY_MASK;
		/*
		 * Nested types are accessed from the nesting name.  We use the fact that nested types use different visibility flags
		 * than toplevel types, thus avoiding the need to grovel through the NESTED_TYPE table
		 */
		if (visib >= TYPE_ATTRIBUTE_NESTED_PUBLIC && visib <= TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM)
			continue;
		name = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAME]);
		nspace = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAMESPACE]);

		nspace_index = cols [MONO_TYPEDEF_NAMESPACE];
		nspace_table = (GHashTable *)g_hash_table_lookup (name_cache2, GUINT_TO_POINTER (nspace_index));
		if (!nspace_table) {
			nspace_table = g_hash_table_new (g_str_hash, g_str_equal);
			g_hash_table_insert (the_name_cache, (char*)nspace, nspace_table);
			g_hash_table_insert (name_cache2, GUINT_TO_POINTER (nspace_index),
								 nspace_table);
		}
		g_hash_table_insert (nspace_table, (char *) name, GUINT_TO_POINTER (i));
	}

	/* Load type names from EXPORTEDTYPES table */
	{
		MonoTableInfo  *t = &image->tables [MONO_TABLE_EXPORTEDTYPE];
		guint32 cols [MONO_EXP_TYPE_SIZE];
		int i;

		for (i = 0; i < t->rows; ++i) {
			mono_metadata_decode_row (t, i, cols, MONO_EXP_TYPE_SIZE);

			guint32 impl = cols [MONO_EXP_TYPE_IMPLEMENTATION];
			if ((impl & MONO_IMPLEMENTATION_MASK) == MONO_IMPLEMENTATION_EXP_TYPE)
				/* Nested type */
				continue;

			name = mono_metadata_string_heap (image, cols [MONO_EXP_TYPE_NAME]);
			nspace = mono_metadata_string_heap (image, cols [MONO_EXP_TYPE_NAMESPACE]);

			nspace_index = cols [MONO_EXP_TYPE_NAMESPACE];
			nspace_table = (GHashTable *)g_hash_table_lookup (name_cache2, GUINT_TO_POINTER (nspace_index));
			if (!nspace_table) {
				nspace_table = g_hash_table_new (g_str_hash, g_str_equal);
				g_hash_table_insert (the_name_cache, (char*)nspace, nspace_table);
				g_hash_table_insert (name_cache2, GUINT_TO_POINTER (nspace_index),
									 nspace_table);
			}
			g_hash_table_insert (nspace_table, (char *) name, GUINT_TO_POINTER (mono_metadata_make_token (MONO_TABLE_EXPORTEDTYPE, i + 1)));
		}
	}

	g_hash_table_destroy (name_cache2);

	mono_image_lock (image);
	if (image->name_cache) {
		/* Somebody initialized it before us */
		g_hash_table_destroy (the_name_cache);
	} else {
		mono_atomic_store_release (&image->name_cache, the_name_cache);
	}
	mono_image_unlock (image);
}

/*FIXME Only dynamic assemblies should allow this operation.*/
/**
 * mono_image_add_to_name_cache:
 */
void
mono_image_add_to_name_cache (MonoImage *image, const char *nspace, 
							  const char *name, guint32 index)
{
	GHashTable *nspace_table;
	GHashTable *name_cache;
	guint32 old_index;

	mono_image_init_name_cache (image);
	mono_image_lock (image);

	name_cache = image->name_cache;
	if (!(nspace_table = (GHashTable *)g_hash_table_lookup (name_cache, nspace))) {
		nspace_table = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (name_cache, (char *)nspace, (char *)nspace_table);
	}

	if ((old_index = GPOINTER_TO_UINT (g_hash_table_lookup (nspace_table, (char*) name))))
		g_error ("overrwritting old token %x on image %s for type %s::%s", old_index, image->name, nspace, name);

	g_hash_table_insert (nspace_table, (char *) name, GUINT_TO_POINTER (index));

	mono_image_unlock (image);
}

typedef struct {
	gconstpointer key;
	gpointer value;
} FindUserData;

static void
find_nocase (gpointer key, gpointer value, gpointer user_data)
{
	char *name = (char*)key;
	FindUserData *data = (FindUserData*)user_data;

	if (!data->value && (mono_utf8_strcasecmp (name, (char*)data->key) == 0))
		data->value = value;
}

/**
 * mono_class_from_name_case:
 * \param image The MonoImage where the type is looked up in
 * \param name_space the type namespace
 * \param name the type short name.
 * \deprecated use the mono_class_from_name_case_checked variant instead.
 *
 * Obtains a \c MonoClass with a given namespace and a given name which
 * is located in the given \c MonoImage.   The namespace and name
 * lookups are case insensitive.
 */
MonoClass *
mono_class_from_name_case (MonoImage *image, const char* name_space, const char *name)
{
	ERROR_DECL (error);
	MonoClass *res = mono_class_from_name_case_checked (image, name_space, name, error);
	mono_error_cleanup (error);

	return res;
}

/**
 * mono_class_from_name_case_checked:
 * \param image The MonoImage where the type is looked up in
 * \param name_space the type namespace
 * \param name the type short name.
 * \param error if 
 *
 * Obtains a MonoClass with a given namespace and a given name which
 * is located in the given MonoImage.   The namespace and name
 * lookups are case insensitive.
 *
 * \returns The MonoClass if the given namespace and name were found, or NULL if it
 * was not found.   The \p error object will contain information about the problem
 * in that case.
 */
MonoClass *
mono_class_from_name_case_checked (MonoImage *image, const char *name_space, const char *name, MonoError *error)
{
	MonoTableInfo  *t = &image->tables [MONO_TABLE_TYPEDEF];
	guint32 cols [MONO_TYPEDEF_SIZE];
	const char *n;
	const char *nspace;
	guint32 i, visib;

	error_init (error);

	if (image_is_dynamic (image)) {
		guint32 token = 0;
		FindUserData user_data;

		mono_image_init_name_cache (image);
		mono_image_lock (image);

		user_data.key = name_space;
		user_data.value = NULL;
		g_hash_table_foreach (image->name_cache, find_nocase, &user_data);

		if (user_data.value) {
			GHashTable *nspace_table = (GHashTable*)user_data.value;

			user_data.key = name;
			user_data.value = NULL;

			g_hash_table_foreach (nspace_table, find_nocase, &user_data);
			
			if (user_data.value)
				token = GPOINTER_TO_UINT (user_data.value);
		}

		mono_image_unlock (image);
		
		if (token)
			return mono_class_get_checked (image, MONO_TOKEN_TYPE_DEF | token, error);
		else
			return NULL;

	}

	/* add a cache if needed */
	for (i = 1; i <= t->rows; ++i) {
		mono_metadata_decode_row (t, i - 1, cols, MONO_TYPEDEF_SIZE);
		visib = cols [MONO_TYPEDEF_FLAGS] & TYPE_ATTRIBUTE_VISIBILITY_MASK;
		/*
		 * Nested types are accessed from the nesting name.  We use the fact that nested types use different visibility flags
		 * than toplevel types, thus avoiding the need to grovel through the NESTED_TYPE table
		 */
		if (visib >= TYPE_ATTRIBUTE_NESTED_PUBLIC && visib <= TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM)
			continue;
		n = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAME]);
		nspace = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAMESPACE]);
		if (mono_utf8_strcasecmp (n, name) == 0 && mono_utf8_strcasecmp (nspace, name_space) == 0)
			return mono_class_get_checked (image, MONO_TOKEN_TYPE_DEF | i, error);
	}
	return NULL;
}

static MonoClass*
return_nested_in (MonoClass *klass, char *nested)
{
	MonoClass *found;
	char *s = strchr (nested, '/');
	gpointer iter = NULL;

	if (s) {
		*s = 0;
		s++;
	}

	while ((found = mono_class_get_nested_types (klass, &iter))) {
		if (strcmp (found->name, nested) == 0) {
			if (s)
				return return_nested_in (found, s);
			return found;
		}
	}
	return NULL;
}

static MonoClass*
search_modules (MonoImage *image, const char *name_space, const char *name, MonoError *error)
{
	MonoTableInfo *file_table = &image->tables [MONO_TABLE_FILE];
	MonoImage *file_image;
	MonoClass *klass;
	int i;

	error_init (error);

	/* 
	 * The EXPORTEDTYPES table only contains public types, so have to search the
	 * modules as well.
	 * Note: image->modules contains the contents of the MODULEREF table, while
	 * the real module list is in the FILE table.
	 */
	for (i = 0; i < file_table->rows; i++) {
		guint32 cols [MONO_FILE_SIZE];
		mono_metadata_decode_row (file_table, i, cols, MONO_FILE_SIZE);
		if (cols [MONO_FILE_FLAGS] == FILE_CONTAINS_NO_METADATA)
			continue;

		file_image = mono_image_load_file_for_image_checked (image, i + 1, error);
		if (file_image) {
			klass = mono_class_from_name_checked (file_image, name_space, name, error);
			if (klass || !is_ok (error))
				return klass;
		}
	}

	return NULL;
}

static MonoClass *
mono_class_from_name_checked_aux (MonoImage *image, const char* name_space, const char *name, GHashTable* visited_images, MonoError *error)
{
	GHashTable *nspace_table;
	MonoImage *loaded_image;
	guint32 token = 0;
	int i;
	MonoClass *klass;
	char *nested;
	char buf [1024];

	error_init (error);

	// Checking visited images avoids stack overflows when cyclic references exist.
	if (g_hash_table_lookup (visited_images, image))
		return NULL;

	g_hash_table_insert (visited_images, image, GUINT_TO_POINTER(1));

	if ((nested = strchr (name, '/'))) {
		int pos = nested - name;
		int len = strlen (name);
		if (len > 1023)
			return NULL;
		memcpy (buf, name, len + 1);
		buf [pos] = 0;
		nested = buf + pos + 1;
		name = buf;
	}

	/* FIXME: get_class_from_name () can't handle types in the EXPORTEDTYPE table */
	if (get_class_from_name && image->tables [MONO_TABLE_EXPORTEDTYPE].rows == 0) {
		gboolean res = get_class_from_name (image, name_space, name, &klass);
		if (res) {
			if (!klass) {
				klass = search_modules (image, name_space, name, error);
				if (!is_ok (error))
					return NULL;
			}
			if (nested)
				return klass ? return_nested_in (klass, nested) : NULL;
			else
				return klass;
		}
	}

	mono_image_init_name_cache (image);
	mono_image_lock (image);

	nspace_table = (GHashTable *)g_hash_table_lookup (image->name_cache, name_space);

	if (nspace_table)
		token = GPOINTER_TO_UINT (g_hash_table_lookup (nspace_table, name));

	mono_image_unlock (image);

	if (!token && image_is_dynamic (image) && image->modules) {
		/* Search modules as well */
		for (i = 0; i < image->module_count; ++i) {
			MonoImage *module = image->modules [i];

			klass = mono_class_from_name_checked (module, name_space, name, error);
			if (klass || !is_ok (error))
				return klass;
		}
	}

	if (!token) {
		klass = search_modules (image, name_space, name, error);
		if (klass || !is_ok (error))
			return klass;
		return NULL;
	}

	if (mono_metadata_token_table (token) == MONO_TABLE_EXPORTEDTYPE) {
		MonoTableInfo  *t = &image->tables [MONO_TABLE_EXPORTEDTYPE];
		guint32 cols [MONO_EXP_TYPE_SIZE];
		guint32 idx, impl;

		idx = mono_metadata_token_index (token);

		mono_metadata_decode_row (t, idx - 1, cols, MONO_EXP_TYPE_SIZE);

		impl = cols [MONO_EXP_TYPE_IMPLEMENTATION];
		if ((impl & MONO_IMPLEMENTATION_MASK) == MONO_IMPLEMENTATION_FILE) {
			loaded_image = mono_assembly_load_module_checked (image->assembly, impl >> MONO_IMPLEMENTATION_BITS, error);
			if (!loaded_image)
				return NULL;
			klass = mono_class_from_name_checked_aux (loaded_image, name_space, name, visited_images, error);
			if (nested)
				return klass ? return_nested_in (klass, nested) : NULL;
			return klass;
		} else if ((impl & MONO_IMPLEMENTATION_MASK) == MONO_IMPLEMENTATION_ASSEMBLYREF) {
			guint32 assembly_idx;

			assembly_idx = impl >> MONO_IMPLEMENTATION_BITS;

			mono_assembly_load_reference (image, assembly_idx - 1);
			g_assert (image->references [assembly_idx - 1]);
			if (image->references [assembly_idx - 1] == (gpointer)-1)
				return NULL;			
			klass = mono_class_from_name_checked_aux (image->references [assembly_idx - 1]->image, name_space, name, visited_images, error);
			if (nested)
				return klass ? return_nested_in (klass, nested) : NULL;
			return klass;
		} else {
			g_assert_not_reached ();
		}
	}

	token = MONO_TOKEN_TYPE_DEF | token;

	klass = mono_class_get_checked (image, token, error);
	if (nested)
		return return_nested_in (klass, nested);
	return klass;
}

/**
 * mono_class_from_name_checked:
 * \param image The MonoImage where the type is looked up in
 * \param name_space the type namespace
 * \param name the type short name.
 *
 * Obtains a MonoClass with a given namespace and a given name which
 * is located in the given MonoImage.
 *
 * Works like mono_class_from_name, but error handling is tricky. It can return NULL and have no error
 * set if the class was not found or it will return NULL and set the error if there was a loading error.
 */
MonoClass *
mono_class_from_name_checked (MonoImage *image, const char* name_space, const char *name, MonoError *error)
{
	MonoClass *klass;
	GHashTable *visited_images;

	visited_images = g_hash_table_new (g_direct_hash, g_direct_equal);

	klass = mono_class_from_name_checked_aux (image, name_space, name, visited_images, error);

	g_hash_table_destroy (visited_images);

	return klass;
}

/**
 * mono_class_from_name:
 * \param image The \c MonoImage where the type is looked up in
 * \param name_space the type namespace
 * \param name the type short name.
 *
 * Obtains a \c MonoClass with a given namespace and a given name which
 * is located in the given \c MonoImage.
 *
 * To reference nested classes, use the "/" character as a separator.
 * For example use \c "Foo/Bar" to reference the class \c Bar that is nested
 * inside \c Foo, like this: "class Foo { class Bar {} }".
 */
MonoClass *
mono_class_from_name (MonoImage *image, const char* name_space, const char *name)
{
	ERROR_DECL (error);
	MonoClass *klass;

	klass = mono_class_from_name_checked (image, name_space, name, error);
	mono_error_cleanup (error); /* FIXME Don't swallow the error */

	return klass;
}

/**
 * mono_class_load_from_name:
 * \param image The MonoImage where the type is looked up in
 * \param name_space the type namespace
 * \param name the type short name.
 *
 * This function works exactly like mono_class_from_name but it will abort if the class is not found.
 * This function should be used by the runtime for critical types to which there's no way to recover but crash
 * If they are missing. Thing of System.Object or System.String.
 */
MonoClass *
mono_class_load_from_name (MonoImage *image, const char* name_space, const char *name)
{
	ERROR_DECL (error);
	MonoClass *klass;

	klass = mono_class_from_name_checked (image, name_space, name, error);
	if (!klass)
		g_error ("Runtime critical type %s.%s not found", name_space, name);
	mono_error_assertf_ok (error, "Could not load runtime critical type %s.%s", name_space, name);
	return klass;
}

/**
 * mono_class_try_load_from_name:
 * \param image The MonoImage where the type is looked up in
 * \param name_space the type namespace
 * \param name the type short name.
 *
 * This function tries to load a type, returning the class was found or NULL otherwise.
 * This function should be used by the runtime when probing for optional types, those that could have being linked out.
 *
 * Big design consideration. This function aborts if there was an error loading the type. This prevents us from missing
 * a type that we would otherwise assume to be available but was not due some error.
 *
 */
MonoClass*
mono_class_try_load_from_name (MonoImage *image, const char* name_space, const char *name)
{
	ERROR_DECL (error);
	MonoClass *klass;

	klass = mono_class_from_name_checked (image, name_space, name, error);
	mono_error_assertf_ok (error, "Could not load runtime critical type %s.%s", name_space, name);
	return klass;
}


/**
 * mono_class_is_subclass_of:
 * \param klass class to probe if it is a subclass of another one
 * \param klassc the class we suspect is the base class
 * \param check_interfaces whether we should perform interface checks
 *
 * This method determines whether \p klass is a subclass of \p klassc.
 *
 * If the \p check_interfaces flag is set, then if \p klassc is an interface
 * this method return TRUE if the \p klass implements the interface or
 * if \p klass is an interface, if one of its base classes is \p klass.
 *
 * If \p check_interfaces is false, then if \p klass is not an interface,
 * it returns TRUE if the \p klass is a subclass of \p klassc.
 *
 * if \p klass is an interface and \p klassc is \c System.Object, then this function
 * returns TRUE.
 *
 */
gboolean
mono_class_is_subclass_of (MonoClass *klass, MonoClass *klassc, 
			   gboolean check_interfaces)
{
	/* FIXME test for interfaces with variant generic arguments */
	mono_class_init (klass);
	mono_class_init (klassc);
	
	if (check_interfaces && MONO_CLASS_IS_INTERFACE (klassc) && !MONO_CLASS_IS_INTERFACE (klass)) {
		if (MONO_CLASS_IMPLEMENTS_INTERFACE (klass, klassc->interface_id))
			return TRUE;
	} else if (check_interfaces && MONO_CLASS_IS_INTERFACE (klassc) && MONO_CLASS_IS_INTERFACE (klass)) {
		int i;

		for (i = 0; i < klass->interface_count; i ++) {
			MonoClass *ic =  klass->interfaces [i];
			if (ic == klassc)
				return TRUE;
		}
	} else {
		if (!MONO_CLASS_IS_INTERFACE (klass) && mono_class_has_parent (klass, klassc))
			return TRUE;
	}

	/* 
	 * MS.NET thinks interfaces are a subclass of Object, so we think it as
	 * well.
	 */
	if (klassc == mono_defaults.object_class)
		return TRUE;

	return FALSE;
}

static gboolean
mono_type_is_generic_argument (MonoType *type)
{
	return type->type == MONO_TYPE_VAR || type->type == MONO_TYPE_MVAR;
}

gboolean
mono_class_has_variant_generic_params (MonoClass *klass)
{
	int i;
	MonoGenericContainer *container;

	if (!mono_class_is_ginst (klass))
		return FALSE;

	container = mono_class_get_generic_container (mono_class_get_generic_class (klass)->container_class);

	for (i = 0; i < container->type_argc; ++i)
		if (mono_generic_container_get_param_info (container, i)->flags & (MONO_GEN_PARAM_VARIANT|MONO_GEN_PARAM_COVARIANT))
			return TRUE;

	return FALSE;
}

static gboolean
mono_gparam_is_reference_conversible (MonoClass *target, MonoClass *candidate, gboolean check_for_reference_conv)
{
	if (target == candidate)
		return TRUE;

	if (check_for_reference_conv &&
		mono_type_is_generic_argument (&target->byval_arg) &&
		mono_type_is_generic_argument (&candidate->byval_arg)) {
		MonoGenericParam *gparam = candidate->byval_arg.data.generic_param;
		MonoGenericParamInfo *pinfo = mono_generic_param_info (gparam);

		if (!pinfo || (pinfo->flags & GENERIC_PARAMETER_ATTRIBUTE_REFERENCE_TYPE_CONSTRAINT) == 0)
			return FALSE;
	}
	if (!mono_class_is_assignable_from (target, candidate))
		return FALSE;
	return TRUE;
}

/**
 * @container the generic container from the GTD
 * @klass: the class to be assigned to
 * @oklass: the source class
 * 
 * Both @klass and @oklass must be instances of the same generic interface.
 *
 * Returns: TRUE if @klass can be assigned to a @klass variable
 */
gboolean
mono_class_is_variant_compatible (MonoClass *klass, MonoClass *oklass, gboolean check_for_reference_conv)
{
	int j;
	MonoType **klass_argv, **oklass_argv;
	MonoClass *klass_gtd = mono_class_get_generic_type_definition (klass);
	MonoGenericContainer *container = mono_class_get_generic_container (klass_gtd);

	if (klass == oklass)
		return TRUE;

	/*Viable candidates are instances of the same generic interface*/
	if (mono_class_get_generic_type_definition (oklass) != klass_gtd || oklass == klass_gtd)
		return FALSE;

	klass_argv = &mono_class_get_generic_class (klass)->context.class_inst->type_argv [0];
	oklass_argv = &mono_class_get_generic_class (oklass)->context.class_inst->type_argv [0];

	for (j = 0; j < container->type_argc; ++j) {
		MonoClass *param1_class = mono_class_from_mono_type (klass_argv [j]);
		MonoClass *param2_class = mono_class_from_mono_type (oklass_argv [j]);

		if (param1_class->valuetype != param2_class->valuetype || (param1_class->valuetype && param1_class != param2_class))
			return FALSE;

		/*
		 * The _VARIANT and _COVARIANT constants should read _COVARIANT and
		 * _CONTRAVARIANT, but they are in a public header so we can't fix it.
		 */
		if (param1_class != param2_class) {
			if (mono_generic_container_get_param_info (container, j)->flags & MONO_GEN_PARAM_VARIANT) {
				if (!mono_gparam_is_reference_conversible (param1_class, param2_class, check_for_reference_conv))
					return FALSE;
			} else if (mono_generic_container_get_param_info (container, j)->flags & MONO_GEN_PARAM_COVARIANT) {
				if (!mono_gparam_is_reference_conversible (param2_class, param1_class, check_for_reference_conv))
					return FALSE;
			} else
				return FALSE;
		}
	}
	return TRUE;
}

static gboolean
mono_gparam_is_assignable_from (MonoClass *target, MonoClass *candidate)
{
	MonoGenericParam *gparam, *ogparam;
	MonoGenericParamInfo *tinfo, *cinfo;
	MonoClass **candidate_class;
	gboolean class_constraint_satisfied, valuetype_constraint_satisfied;
	int tmask, cmask;

	if (target == candidate)
		return TRUE;
	if (target->byval_arg.type != candidate->byval_arg.type)
		return FALSE;

	gparam = target->byval_arg.data.generic_param;
	ogparam = candidate->byval_arg.data.generic_param;
	tinfo = mono_generic_param_info (gparam);
	cinfo = mono_generic_param_info (ogparam);

	class_constraint_satisfied = FALSE;
	valuetype_constraint_satisfied = FALSE;

	/*candidate must have a super set of target's special constraints*/
	tmask = tinfo->flags & GENERIC_PARAMETER_ATTRIBUTE_SPECIAL_CONSTRAINTS_MASK;
	cmask = cinfo->flags & GENERIC_PARAMETER_ATTRIBUTE_SPECIAL_CONSTRAINTS_MASK;

	if (cinfo->constraints) {
		for (candidate_class = cinfo->constraints; *candidate_class; ++candidate_class) {
			MonoClass *cc = *candidate_class;

			if (mono_type_is_reference (&cc->byval_arg) && !MONO_CLASS_IS_INTERFACE (cc))
				class_constraint_satisfied = TRUE;
			else if (!mono_type_is_reference (&cc->byval_arg) && !MONO_CLASS_IS_INTERFACE (cc))
				valuetype_constraint_satisfied = TRUE;
		}
	}
	class_constraint_satisfied |= (cmask & GENERIC_PARAMETER_ATTRIBUTE_REFERENCE_TYPE_CONSTRAINT) != 0;
	valuetype_constraint_satisfied |= (cmask & GENERIC_PARAMETER_ATTRIBUTE_VALUE_TYPE_CONSTRAINT) != 0;

	if ((tmask & GENERIC_PARAMETER_ATTRIBUTE_REFERENCE_TYPE_CONSTRAINT) && !class_constraint_satisfied)
		return FALSE;
	if ((tmask & GENERIC_PARAMETER_ATTRIBUTE_VALUE_TYPE_CONSTRAINT) && !valuetype_constraint_satisfied)
		return FALSE;
	if ((tmask & GENERIC_PARAMETER_ATTRIBUTE_CONSTRUCTOR_CONSTRAINT) && !((cmask & GENERIC_PARAMETER_ATTRIBUTE_CONSTRUCTOR_CONSTRAINT) ||
		valuetype_constraint_satisfied)) {
		return FALSE;
	}


	/*candidate type constraints must be a superset of target's*/
	if (tinfo->constraints) {
		MonoClass **target_class;
		for (target_class = tinfo->constraints; *target_class; ++target_class) {
			MonoClass *tc = *target_class;

			/*
			 * A constraint from @target might inflate into @candidate itself and in that case we don't need
			 * check it's constraints since it satisfy the constraint by itself.
			 */
			if (mono_metadata_type_equal (&tc->byval_arg, &candidate->byval_arg))
				continue;

			if (!cinfo->constraints)
				return FALSE;

			for (candidate_class = cinfo->constraints; *candidate_class; ++candidate_class) {
				MonoClass *cc = *candidate_class;

				if (mono_class_is_assignable_from (tc, cc))
					break;

				/*
				 * This happens when we have the following:
				 *
				 * Bar<K> where K : IFace
				 * Foo<T, U> where T : U where U : IFace
				 * 	...
				 * 	Bar<T> <- T here satisfy K constraint transitively through to U's constraint
				 *
				 */
				if (mono_type_is_generic_argument (&cc->byval_arg)) {
					if (mono_gparam_is_assignable_from (target, cc))
						break;
				}
			}
			if (!*candidate_class)
				return FALSE;
		}
	}

	/*candidate itself must have a constraint that satisfy target*/
	if (cinfo->constraints) {
		for (candidate_class = cinfo->constraints; *candidate_class; ++candidate_class) {
			MonoClass *cc = *candidate_class;
			if (mono_class_is_assignable_from (target, cc))
				return TRUE;
		}
	}
	return FALSE;
}

/**
 * mono_class_is_assignable_from:
 * \param klass the class to be assigned to
 * \param oklass the source class
 *
 * \returns TRUE if an instance of class \p oklass can be assigned to an
 * instance of class \p klass
 */
gboolean
mono_class_is_assignable_from (MonoClass *klass, MonoClass *oklass)
{
	ERROR_DECL (error);
	/*FIXME this will cause a lot of irrelevant stuff to be loaded.*/
	if (!klass->inited)
		mono_class_init (klass);

	if (!oklass->inited)
		mono_class_init (oklass);

	if (mono_class_has_failure (klass) || mono_class_has_failure  (oklass))
		return FALSE;

	if (mono_type_is_generic_argument (&klass->byval_arg)) {
		if (!mono_type_is_generic_argument (&oklass->byval_arg))
			return FALSE;
		return mono_gparam_is_assignable_from (klass, oklass);
	}

	/* This can happen if oklass is a tyvar that has a constraint which is another tyvar which in turn
	 * has a constraint which is a class type:
	 *
	 *  class Foo { }
	 *  class G<T1, T2> where T1 : T2 where T2 : Foo { }
	 *
	 * In this case, Foo is assignable from T1.
	 */
	if ((oklass->byval_arg.type == MONO_TYPE_VAR) || (oklass->byval_arg.type == MONO_TYPE_MVAR)) {
		MonoGenericParam *gparam = oklass->byval_arg.data.generic_param;
		MonoClass **constraints = mono_generic_container_get_param_info (gparam->owner, gparam->num)->constraints;
		int i;

		if (constraints) {
			for (i = 0; constraints [i]; ++i) {
				if (mono_class_is_assignable_from (klass, constraints [i]))
					return TRUE;
			}
		}

		return mono_class_has_parent (oklass, klass);
	}

	if (MONO_CLASS_IS_INTERFACE (klass)) {

		/* interface_offsets might not be set for dynamic classes */
		if (mono_class_get_ref_info_handle (oklass) && !oklass->interface_bitmap) {
			/* 
			 * oklass might be a generic type parameter but they have 
			 * interface_offsets set.
			 */
			gboolean result = mono_reflection_call_is_assignable_to (oklass, klass, error);
			if (!is_ok (error)) {
				mono_error_cleanup (error);
				return FALSE;
			}
			return result;
		}
		if (!oklass->interface_bitmap)
			/* Happens with generic instances of not-yet created dynamic types */
			return FALSE;
		if (MONO_CLASS_IMPLEMENTS_INTERFACE (oklass, klass->interface_id))
			return TRUE;

		if (klass->is_array_special_interface && oklass->rank == 1) {
			//XXX we could offset this by having the cast target computed at JIT time
			//XXX we could go even further and emit a wrapper that would do the extra type check
			MonoClass *iface_klass = mono_class_from_mono_type (mono_class_get_generic_class (klass)->context.class_inst->type_argv [0]);
			MonoClass *obj_klass = oklass->cast_class; //This gets us the cast class of element type of the array

			// If the target we're trying to cast to is a valuetype, we must account of weird valuetype equivalences such as IntEnum <> int or uint <> int
			// We can't apply it for ref types as this would go wrong with arrays - IList<byte[]> would have byte tested
			if (!mono_class_is_nullable (iface_klass)) {
				if (iface_klass->valuetype)
					iface_klass = iface_klass->cast_class;

				//array covariant casts only operates on scalar to scalar
				//This is so int[] can't be casted to IComparable<int>[]
				if (!(obj_klass->valuetype && !iface_klass->valuetype) && mono_class_is_assignable_from (iface_klass, obj_klass))
					return TRUE;
			}
		}

		if (mono_class_has_variant_generic_params (klass)) {
			int i;
			mono_class_setup_interfaces (oklass, error);
			if (!mono_error_ok (error)) {
				mono_error_cleanup (error);
				return FALSE;
			}

			/*klass is a generic variant interface, We need to extract from oklass a list of ifaces which are viable candidates.*/
			for (i = 0; i < oklass->interface_offsets_count; ++i) {
				MonoClass *iface = oklass->interfaces_packed [i];

				if (mono_class_is_variant_compatible (klass, iface, FALSE))
					return TRUE;
			}
		}
		return FALSE;
	} else if (klass->delegate) {
		if (mono_class_has_variant_generic_params (klass) && mono_class_is_variant_compatible (klass, oklass, FALSE))
			return TRUE;
	}else if (klass->rank) {
		MonoClass *eclass, *eoclass;

		if (oklass->rank != klass->rank)
			return FALSE;

		/* vectors vs. one dimensional arrays */
		if (oklass->byval_arg.type != klass->byval_arg.type)
			return FALSE;

		eclass = klass->cast_class;
		eoclass = oklass->cast_class;

		/* 
		 * a is b does not imply a[] is b[] when a is a valuetype, and
		 * b is a reference type.
		 */

		if (eoclass->valuetype) {
			if ((eclass == mono_defaults.enum_class) || 
				(eclass == mono_defaults.enum_class->parent) ||
				(eclass == mono_defaults.object_class))
				return FALSE;
		}

		return mono_class_is_assignable_from (klass->cast_class, oklass->cast_class);
	} else if (mono_class_is_nullable (klass)) {
		if (mono_class_is_nullable (oklass))
			return mono_class_is_assignable_from (klass->cast_class, oklass->cast_class);
		else
			return mono_class_is_assignable_from (klass->cast_class, oklass);
	} else if (klass == mono_defaults.object_class)
		return TRUE;

	return mono_class_has_parent (oklass, klass);
}	

/*Check if @oklass is variant compatible with @klass.*/
static gboolean
mono_class_is_variant_compatible_slow (MonoClass *klass, MonoClass *oklass)
{
	int j;
	MonoType **klass_argv, **oklass_argv;
	MonoClass *klass_gtd = mono_class_get_generic_type_definition (klass);
	MonoGenericContainer *container = mono_class_get_generic_container (klass_gtd);

	/*Viable candidates are instances of the same generic interface*/
	if (mono_class_get_generic_type_definition (oklass) != klass_gtd || oklass == klass_gtd)
		return FALSE;

	klass_argv = &mono_class_get_generic_class (klass)->context.class_inst->type_argv [0];
	oklass_argv = &mono_class_get_generic_class (oklass)->context.class_inst->type_argv [0];

	for (j = 0; j < container->type_argc; ++j) {
		MonoClass *param1_class = mono_class_from_mono_type (klass_argv [j]);
		MonoClass *param2_class = mono_class_from_mono_type (oklass_argv [j]);

		if (param1_class->valuetype != param2_class->valuetype)
			return FALSE;

		/*
		 * The _VARIANT and _COVARIANT constants should read _COVARIANT and
		 * _CONTRAVARIANT, but they are in a public header so we can't fix it.
		 */
		if (param1_class != param2_class) {
			if (mono_generic_container_get_param_info (container, j)->flags & MONO_GEN_PARAM_VARIANT) {
				if (!mono_class_is_assignable_from_slow (param1_class, param2_class))
					return FALSE;
			} else if (mono_generic_container_get_param_info (container, j)->flags & MONO_GEN_PARAM_COVARIANT) {
				if (!mono_class_is_assignable_from_slow (param2_class, param1_class))
					return FALSE;
			} else
				return FALSE;
		}
	}
	return TRUE;
}
/*Check if @candidate implements the interface @target*/
static gboolean
mono_class_implement_interface_slow (MonoClass *target, MonoClass *candidate)
{
	ERROR_DECL (error);
	int i;
	gboolean is_variant = mono_class_has_variant_generic_params (target);

	if (is_variant && MONO_CLASS_IS_INTERFACE (candidate)) {
		if (mono_class_is_variant_compatible_slow (target, candidate))
			return TRUE;
	}

	do {
		if (candidate == target)
			return TRUE;

		/*A TypeBuilder can have more interfaces on tb->interfaces than on candidate->interfaces*/
		if (image_is_dynamic (candidate->image) && !candidate->wastypebuilder) {
			MonoReflectionTypeBuilder *tb = (MonoReflectionTypeBuilder *)mono_class_get_ref_info_raw (candidate); /* FIXME use handles */
			int j;
			if (tb && tb->interfaces) {
				for (j = mono_array_length (tb->interfaces) - 1; j >= 0; --j) {
					MonoReflectionType *iface = mono_array_get (tb->interfaces, MonoReflectionType*, j);
					MonoClass *iface_class;

					/* we can't realize the type here since it can do pretty much anything. */
					if (!iface->type)
						continue;
					iface_class = mono_class_from_mono_type (iface->type);
					if (iface_class == target)
						return TRUE;
					if (is_variant && mono_class_is_variant_compatible_slow (target, iface_class))
						return TRUE;
					if (mono_class_implement_interface_slow (target, iface_class))
						return TRUE;
				}
			}
		} else {
			/*setup_interfaces don't mono_class_init anything*/
			/*FIXME this doesn't handle primitive type arrays.
			ICollection<sbyte> x byte [] won't work because candidate->interfaces, for byte[], won't have IList<sbyte>.
			A possible way to fix this would be to move that to setup_interfaces from setup_interface_offsets.
			*/
			mono_class_setup_interfaces (candidate, error);
			if (!mono_error_ok (error)) {
				mono_error_cleanup (error);
				return FALSE;
			}

			for (i = 0; i < candidate->interface_count; ++i) {
				if (candidate->interfaces [i] == target)
					return TRUE;
				
				if (is_variant && mono_class_is_variant_compatible_slow (target, candidate->interfaces [i]))
					return TRUE;

				 if (mono_class_implement_interface_slow (target, candidate->interfaces [i]))
					return TRUE;
			}
		}
		candidate = candidate->parent;
	} while (candidate);

	return FALSE;
}

/*
 * Check if @oklass can be assigned to @klass.
 * This function does the same as mono_class_is_assignable_from but is safe to be used from mono_class_init context.
 */
gboolean
mono_class_is_assignable_from_slow (MonoClass *target, MonoClass *candidate)
{
	if (candidate == target)
		return TRUE;
	if (target == mono_defaults.object_class)
		return TRUE;

	if (mono_class_has_parent (candidate, target))
		return TRUE;

	/*If target is not an interface there is no need to check them.*/
	if (MONO_CLASS_IS_INTERFACE (target))
		return mono_class_implement_interface_slow (target, candidate);

 	if (target->delegate && mono_class_has_variant_generic_params (target))
		return mono_class_is_variant_compatible (target, candidate, FALSE);

	if (target->rank) {
		MonoClass *eclass, *eoclass;

		if (target->rank != candidate->rank)
			return FALSE;

		/* vectors vs. one dimensional arrays */
		if (target->byval_arg.type != candidate->byval_arg.type)
			return FALSE;

		eclass = target->cast_class;
		eoclass = candidate->cast_class;

		/*
		 * a is b does not imply a[] is b[] when a is a valuetype, and
		 * b is a reference type.
		 */

		if (eoclass->valuetype) {
			if ((eclass == mono_defaults.enum_class) ||
				(eclass == mono_defaults.enum_class->parent) ||
				(eclass == mono_defaults.object_class))
				return FALSE;
		}

		return mono_class_is_assignable_from_slow (target->cast_class, candidate->cast_class);
	}
	/*FIXME properly handle nullables */
	/*FIXME properly handle (M)VAR */
	return FALSE;
}

/**
 * mono_class_get_cctor:
 * \param klass A MonoClass pointer
 *
 * \returns The static constructor of \p klass if it exists, NULL otherwise.
 */
MonoMethod*
mono_class_get_cctor (MonoClass *klass)
{
	MonoCachedClassInfo cached_info;

	if (image_is_dynamic (klass->image)) {
		/* 
		 * has_cctor is not set for these classes because mono_class_init () is
		 * not run for them.
		 */
		return mono_class_get_method_from_name_flags (klass, ".cctor", -1, METHOD_ATTRIBUTE_SPECIAL_NAME);
	}

	mono_class_init (klass);

	if (!klass->has_cctor)
		return NULL;

	if (mono_class_is_ginst (klass) && !klass->methods) {
		ERROR_DECL (error);
		error_init (error);
		MonoMethod *result = mono_class_get_inflated_method (klass, mono_class_get_cctor (mono_class_get_generic_class (klass)->container_class), error);
		mono_error_assert_msg_ok (error, "Could not lookup inflated class cctor"); /* FIXME do proper error handling */
		return result;
	}

	if (mono_class_get_cached_class_info (klass, &cached_info)) {
		ERROR_DECL (error);
		MonoMethod *result = mono_get_method_checked (klass->image, cached_info.cctor_token, klass, NULL, error);
		mono_error_assert_msg_ok (error, "Could not lookup class cctor from cached metadata");
		return result;
	}

	return mono_class_get_method_from_name_flags (klass, ".cctor", -1, METHOD_ATTRIBUTE_SPECIAL_NAME);
}

/**
 * mono_class_get_finalizer:
 * \param klass: The MonoClass pointer
 *
 * \returns The finalizer method of \p klass if it exists, NULL otherwise.
 */
MonoMethod*
mono_class_get_finalizer (MonoClass *klass)
{
	MonoCachedClassInfo cached_info;

	if (!klass->inited)
		mono_class_init (klass);
	if (!mono_class_has_finalizer (klass))
		return NULL;

	if (mono_class_get_cached_class_info (klass, &cached_info)) {
		ERROR_DECL (error);
		MonoMethod *result = mono_get_method_checked (cached_info.finalize_image, cached_info.finalize_token, NULL, NULL, error);
		mono_error_assert_msg_ok (error, "Could not lookup finalizer from cached metadata");
		return result;
	}else {
		mono_class_setup_vtable (klass);
		return klass->vtable [mono_class_get_object_finalize_slot ()];
	}
}

/**
 * mono_class_needs_cctor_run:
 * \param klass the MonoClass pointer
 * \param caller a MonoMethod describing the caller
 *
 * Determines whenever the class has a static constructor and whenever it
 * needs to be called when executing CALLER.
 */
gboolean
mono_class_needs_cctor_run (MonoClass *klass, MonoMethod *caller)
{
	MonoMethod *method;

	method = mono_class_get_cctor (klass);
	if (method)
		return (method == caller) ? FALSE : TRUE;
	else
		return FALSE;
}

/**
 * mono_class_array_element_size:
 * \param klass
 *
 * \returns The number of bytes an element of type \p klass uses when stored into an array.
 */
gint32
mono_class_array_element_size (MonoClass *klass)
{
	MonoType *type = &klass->byval_arg;
	
handle_enum:
	switch (type->type) {
	case MONO_TYPE_I1:
	case MONO_TYPE_U1:
	case MONO_TYPE_BOOLEAN:
		return 1;
	case MONO_TYPE_I2:
	case MONO_TYPE_U2:
	case MONO_TYPE_CHAR:
		return 2;
	case MONO_TYPE_I4:
	case MONO_TYPE_U4:
	case MONO_TYPE_R4:
		return 4;
	case MONO_TYPE_I:
	case MONO_TYPE_U:
	case MONO_TYPE_PTR:
	case MONO_TYPE_CLASS:
	case MONO_TYPE_STRING:
	case MONO_TYPE_OBJECT:
	case MONO_TYPE_SZARRAY:
	case MONO_TYPE_ARRAY: 
		return sizeof (gpointer);
	case MONO_TYPE_I8:
	case MONO_TYPE_U8:
	case MONO_TYPE_R8:
		return 8;
	case MONO_TYPE_VALUETYPE:
		if (type->data.klass->enumtype) {
			type = mono_class_enum_basetype (type->data.klass);
			klass = klass->element_class;
			goto handle_enum;
		}
		return mono_class_instance_size (klass) - sizeof (MonoObject);
	case MONO_TYPE_GENERICINST:
		type = &type->data.generic_class->container_class->byval_arg;
		goto handle_enum;
	case MONO_TYPE_VAR:
	case MONO_TYPE_MVAR: {
		int align;

		return mono_type_size (type, &align);
	}
	case MONO_TYPE_VOID:
		return 0;
		
	default:
		g_error ("unknown type 0x%02x in mono_class_array_element_size", type->type);
	}
	return -1;
}

/**
 * mono_array_element_size:
 * \param ac pointer to a \c MonoArrayClass
 *
 * \returns The size of single array element.
 *
 * LOCKING: Acquires the loader lock.
 */
gint32
mono_array_element_size (MonoClass *ac)
{
	g_assert (ac->rank);
	if (G_UNLIKELY (!ac->size_inited)) {
		mono_class_setup_fields (ac);
	}
	return ac->sizes.element_size;
}

/**
 * mono_ldtoken:
 */
gpointer
mono_ldtoken (MonoImage *image, guint32 token, MonoClass **handle_class,
	      MonoGenericContext *context)
{
	ERROR_DECL (error);
	gpointer res = mono_ldtoken_checked (image, token, handle_class, context, error);
	mono_error_assert_ok (error);
	return res;
}

gpointer
mono_ldtoken_checked (MonoImage *image, guint32 token, MonoClass **handle_class,
	      MonoGenericContext *context, MonoError *error)
{
	error_init (error);

	if (image_is_dynamic (image)) {
		MonoClass *tmp_handle_class;
		gpointer obj = mono_lookup_dynamic_token_class (image, token, TRUE, &tmp_handle_class, context, error);

		mono_error_assert_ok (error);
		g_assert (tmp_handle_class);
		if (handle_class)
			*handle_class = tmp_handle_class;

		if (tmp_handle_class == mono_defaults.typehandle_class)
			return &((MonoClass*)obj)->byval_arg;
		else
			return obj;
	}

	switch (token & 0xff000000) {
	case MONO_TOKEN_TYPE_DEF:
	case MONO_TOKEN_TYPE_REF:
	case MONO_TOKEN_TYPE_SPEC: {
		MonoType *type;
		if (handle_class)
			*handle_class = mono_defaults.typehandle_class;
		type = mono_type_get_checked (image, token, context, error);
		if (!type)
			return NULL;

		mono_class_init (mono_class_from_mono_type (type));
		/* We return a MonoType* as handle */
		return type;
	}
	case MONO_TOKEN_FIELD_DEF: {
		MonoClass *klass;
		guint32 type = mono_metadata_typedef_from_field (image, mono_metadata_token_index (token));
		if (!type) {
			mono_error_set_bad_image (error, image, "Bad ldtoken %x", token);
			return NULL;
		}
		if (handle_class)
			*handle_class = mono_defaults.fieldhandle_class;
		klass = mono_class_get_and_inflate_typespec_checked (image, MONO_TOKEN_TYPE_DEF | type, context, error);
		if (!klass)
			return NULL;

		mono_class_init (klass);
		return mono_class_get_field (klass, token);
	}
	case MONO_TOKEN_METHOD_DEF:
	case MONO_TOKEN_METHOD_SPEC: {
		MonoMethod *meth;
		meth = mono_get_method_checked (image, token, NULL, context, error);
		if (handle_class)
			*handle_class = mono_defaults.methodhandle_class;
		if (!meth)
			return NULL;

		return meth;
	}
	case MONO_TOKEN_MEMBER_REF: {
		guint32 cols [MONO_MEMBERREF_SIZE];
		const char *sig;
		mono_metadata_decode_row (&image->tables [MONO_TABLE_MEMBERREF], mono_metadata_token_index (token) - 1, cols, MONO_MEMBERREF_SIZE);
		sig = mono_metadata_blob_heap (image, cols [MONO_MEMBERREF_SIGNATURE]);
		mono_metadata_decode_blob_size (sig, &sig);
		if (*sig == 0x6) { /* it's a field */
			MonoClass *klass;
			MonoClassField *field;
			field = mono_field_from_token_checked (image, token, &klass, context, error);
			if (handle_class)
				*handle_class = mono_defaults.fieldhandle_class;
			return field;
		} else {
			MonoMethod *meth;
			meth = mono_get_method_checked (image, token, NULL, context, error);
			if (handle_class)
				*handle_class = mono_defaults.methodhandle_class;
			return meth;
		}
	}
	default:
		mono_error_set_bad_image (error, image, "Bad ldtoken %x", token);
	}
	return NULL;
}

gpointer
mono_lookup_dynamic_token (MonoImage *image, guint32 token, MonoGenericContext *context, MonoError *error)
{
	MonoClass *handle_class;
	error_init (error);
	return mono_reflection_lookup_dynamic_token (image, token, TRUE, &handle_class, context, error);
}

gpointer
mono_lookup_dynamic_token_class (MonoImage *image, guint32 token, gboolean valid_token, MonoClass **handle_class, MonoGenericContext *context, MonoError *error)
{
	return mono_reflection_lookup_dynamic_token (image, token, valid_token, handle_class, context, error);
}

static MonoGetCachedClassInfo get_cached_class_info = NULL;

void
mono_install_get_cached_class_info (MonoGetCachedClassInfo func)
{
	get_cached_class_info = func;
}

gboolean
mono_class_get_cached_class_info (MonoClass *klass, MonoCachedClassInfo *res)
{
	if (!get_cached_class_info)
		return FALSE;
	else
		return get_cached_class_info (klass, res);
}

void
mono_install_get_class_from_name (MonoGetClassFromName func)
{
	get_class_from_name = func;
}

/**
 * mono_class_get_image:
 *
 * Use this method to get the \c MonoImage* where this class came from.
 *
 * \returns The image where this class is defined.
 */
MonoImage*
mono_class_get_image (MonoClass *klass)
{
	return klass->image;
}

/**
 * mono_class_get_element_class:
 * \param klass the \c MonoClass to act on
 *
 * Use this function to get the element class of an array.
 *
 * \returns The element class of an array.
 */
MonoClass*
mono_class_get_element_class (MonoClass *klass)
{
	return klass->element_class;
}

/**
 * mono_class_is_valuetype:
 * \param klass the \c MonoClass to act on
 *
 * Use this method to determine if the provided \c MonoClass* represents a value type,
 * or a reference type.
 *
 * \returns TRUE if the \c MonoClass represents a \c ValueType, FALSE if it represents a reference type.
 */
gboolean
mono_class_is_valuetype (MonoClass *klass)
{
	return klass->valuetype;
}

/**
 * mono_class_is_enum:
 * \param klass the \c MonoClass to act on
 *
 * Use this function to determine if the provided \c MonoClass* represents an enumeration.
 *
 * \returns TRUE if the \c MonoClass represents an enumeration.
 */
gboolean
mono_class_is_enum (MonoClass *klass)
{
	return klass->enumtype;
}

/**
 * mono_class_enum_basetype:
 * \param klass the \c MonoClass to act on
 *
 * Use this function to get the underlying type for an enumeration value.
 * 
 * \returns The underlying type representation for an enumeration.
 */
MonoType*
mono_class_enum_basetype (MonoClass *klass)
{
	if (klass->element_class == klass)
		/* SRE or broken types */
		return NULL;
	else
		return &klass->element_class->byval_arg;
}

/**
 * mono_class_get_parent
 * \param klass the \c MonoClass to act on
 *
 * \returns The parent class for this class.
 */
MonoClass*
mono_class_get_parent (MonoClass *klass)
{
	return klass->parent;
}

/**
 * mono_class_get_nesting_type:
 * \param klass the \c MonoClass to act on
 *
 * Use this function to obtain the class that the provided \c MonoClass* is nested on.
 *
 * If the return is NULL, this indicates that this class is not nested.
 *
 * \returns The container type where this type is nested or NULL if this type is not a nested type.
 */
MonoClass*
mono_class_get_nesting_type (MonoClass *klass)
{
	return klass->nested_in;
}

/**
 * mono_class_get_rank:
 * \param klass the MonoClass to act on
 *
 * \returns The rank for the array (the number of dimensions).
 */
int
mono_class_get_rank (MonoClass *klass)
{
	return klass->rank;
}

/**
 * mono_class_get_name
 * \param klass the \c MonoClass to act on
 *
 * \returns The name of the class.
 */
const char*
mono_class_get_name (MonoClass *klass)
{
	return klass->name;
}

/**
 * mono_class_get_namespace:
 * \param klass the \c MonoClass to act on
 *
 * \returns The namespace of the class.
 */
const char*
mono_class_get_namespace (MonoClass *klass)
{
	return klass->name_space;
}

/**
 * mono_class_get_type:
 * \param klass the \c MonoClass to act on
 *
 * This method returns the internal \c MonoType representation for the class.
 *
 * \returns The \c MonoType from the class.
 */
MonoType*
mono_class_get_type (MonoClass *klass)
{
	return &klass->byval_arg;
}

/**
 * mono_class_get_type_token:
 * \param klass the \c MonoClass to act on
 *
 * This method returns type token for the class.
 *
 * \returns The type token for the class.
 */
guint32
mono_class_get_type_token (MonoClass *klass)
{
  return klass->type_token;
}

/**
 * mono_class_get_byref_type:
 * \param klass the \c MonoClass to act on
 *
 * 
 */
MonoType*
mono_class_get_byref_type (MonoClass *klass)
{
	return &klass->this_arg;
}

/**
 * mono_class_num_fields:
 * \param klass the \c MonoClass to act on
 *
 * \returns The number of static and instance fields in the class.
 */
int
mono_class_num_fields (MonoClass *klass)
{
	return mono_class_get_field_count (klass);
}

/**
 * mono_class_num_methods:
 * \param klass the \c MonoClass to act on
 *
 * \returns The number of methods in the class.
 */
int
mono_class_num_methods (MonoClass *klass)
{
	return mono_class_get_method_count (klass);
}

/**
 * mono_class_num_properties
 * \param klass the \c MonoClass to act on
 *
 * \returns The number of properties in the class.
 */
int
mono_class_num_properties (MonoClass *klass)
{
	mono_class_setup_properties (klass);

	return mono_class_get_property_info (klass)->count;
}

/**
 * mono_class_num_events:
 * \param klass the \c MonoClass to act on
 *
 * \returns The number of events in the class.
 */
int
mono_class_num_events (MonoClass *klass)
{
	mono_class_setup_events (klass);

	return mono_class_get_event_info (klass)->count;
}

/**
 * mono_class_get_fields:
 * \param klass the \c MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the fields in a class.
 *
 * You must pass a \c gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * \returns a \c MonoClassField* on each iteration, or NULL when no more fields are available.
 */
MonoClassField*
mono_class_get_fields (MonoClass* klass, gpointer *iter)
{
	MonoClassField* field;
	if (!iter)
		return NULL;
	if (!*iter) {
		mono_class_setup_fields (klass);
		if (mono_class_has_failure (klass))
			return NULL;
		/* start from the first */
		if (mono_class_get_field_count (klass)) {
			*iter = &klass->fields [0];
			return &klass->fields [0];
		} else {
			/* no fields */
			return NULL;
		}
	}
	field = (MonoClassField *)*iter;
	field++;
	if (field < &klass->fields [mono_class_get_field_count (klass)]) {
		*iter = field;
		return field;
	}
	return NULL;
}

/**
 * mono_class_get_methods:
 * \param klass the \c MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the fields in a class.
 *
 * You must pass a \c gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * \returns a \c MonoMethod on each iteration or NULL when no more methods are available.
 */
MonoMethod*
mono_class_get_methods (MonoClass* klass, gpointer *iter)
{
	MonoMethod** method;
	if (!iter)
		return NULL;
	if (!*iter) {
		mono_class_setup_methods (klass);

		/*
		 * We can't fail lookup of methods otherwise the runtime will burst in flames on all sort of places.
		 * FIXME we should better report this error to the caller
		 */
		if (!klass->methods)
			return NULL;
		/* start from the first */
		if (mono_class_get_method_count (klass)) {
			*iter = &klass->methods [0];
			return klass->methods [0];
		} else {
			/* no method */
			return NULL;
		}
	}
	method = (MonoMethod **)*iter;
	method++;
	if (method < &klass->methods [mono_class_get_method_count (klass)]) {
		*iter = method;
		return *method;
	}
	return NULL;
}

/**
 * mono_class_get_properties:
 * \param klass the \c MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the properties in a class.
 *
 * You must pass a gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * Returns: a \c MonoProperty* on each invocation, or NULL when no more are available.
 */
MonoProperty*
mono_class_get_properties (MonoClass* klass, gpointer *iter)
{
	MonoProperty* property;
	if (!iter)
		return NULL;
	if (!*iter) {
		mono_class_setup_properties (klass);
		MonoClassPropertyInfo *info = mono_class_get_property_info (klass);
		/* start from the first */
		if (info->count) {
			*iter = &info->properties [0];
			return (MonoProperty *)*iter;
		} else {
			/* no fields */
			return NULL;
		}
	}
	property = (MonoProperty *)*iter;
	property++;
	MonoClassPropertyInfo *info = mono_class_get_property_info (klass);
	if (property < &info->properties [info->count]) {
		*iter = property;
		return (MonoProperty *)*iter;
	}
	return NULL;
}

/**
 * mono_class_get_events:
 * \param klass the \c MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the properties in a class.
 *
 * You must pass a \c gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * \returns a \c MonoEvent* on each invocation, or NULL when no more are available.
 */
MonoEvent*
mono_class_get_events (MonoClass* klass, gpointer *iter)
{
	MonoEvent* event;
	if (!iter)
		return NULL;
	if (!*iter) {
		mono_class_setup_events (klass);
		MonoClassEventInfo *info = mono_class_get_event_info (klass);
		/* start from the first */
		if (info->count) {
			*iter = &info->events [0];
			return (MonoEvent *)*iter;
		} else {
			/* no fields */
			return NULL;
		}
	}
	event = (MonoEvent *)*iter;
	event++;
	MonoClassEventInfo *info = mono_class_get_event_info (klass);
	if (event < &info->events [info->count]) {
		*iter = event;
		return (MonoEvent *)*iter;
	}
	return NULL;
}

/**
 * mono_class_get_interfaces
 * \param klass the \c MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the interfaces implemented by this class.
 *
 * You must pass a \c gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * \returns a \c MonoClass* on each invocation, or NULL when no more are available.
 */
MonoClass*
mono_class_get_interfaces (MonoClass* klass, gpointer *iter)
{
	ERROR_DECL (error);
	MonoClass** iface;
	if (!iter)
		return NULL;
	if (!*iter) {
		if (!klass->inited)
			mono_class_init (klass);
		if (!klass->interfaces_inited) {
			mono_class_setup_interfaces (klass, error);
			if (!mono_error_ok (error)) {
				mono_error_cleanup (error);
				return NULL;
			}
		}
		/* start from the first */
		if (klass->interface_count) {
			*iter = &klass->interfaces [0];
			return klass->interfaces [0];
		} else {
			/* no interface */
			return NULL;
		}
	}
	iface = (MonoClass **)*iter;
	iface++;
	if (iface < &klass->interfaces [klass->interface_count]) {
		*iter = iface;
		return *iface;
	}
	return NULL;
}

static void
setup_nested_types (MonoClass *klass)
{
	ERROR_DECL (error);
	GList *classes, *nested_classes, *l;
	int i;

	if (klass->nested_classes_inited)
		return;

	if (!klass->type_token) {
		mono_loader_lock ();
		klass->nested_classes_inited = TRUE;
		mono_loader_unlock ();
		return;
	}

	i = mono_metadata_nesting_typedef (klass->image, klass->type_token, 1);
	classes = NULL;
	while (i) {
		MonoClass* nclass;
		guint32 cols [MONO_NESTED_CLASS_SIZE];
		mono_metadata_decode_row (&klass->image->tables [MONO_TABLE_NESTEDCLASS], i - 1, cols, MONO_NESTED_CLASS_SIZE);
		nclass = mono_class_create_from_typedef (klass->image, MONO_TOKEN_TYPE_DEF | cols [MONO_NESTED_CLASS_NESTED], error);
		if (!mono_error_ok (error)) {
			/*FIXME don't swallow the error message*/
			mono_error_cleanup (error);

			i = mono_metadata_nesting_typedef (klass->image, klass->type_token, i + 1);
			continue;
		}

		classes = g_list_prepend (classes, nclass);

		i = mono_metadata_nesting_typedef (klass->image, klass->type_token, i + 1);
	}

	nested_classes = NULL;
	for (l = classes; l; l = l->next)
		nested_classes = g_list_prepend_image (klass->image, nested_classes, l->data);
	g_list_free (classes);

	mono_loader_lock ();
	if (!klass->nested_classes_inited) {
		mono_class_set_nested_classes_property (klass, nested_classes);
		mono_memory_barrier ();
		klass->nested_classes_inited = TRUE;
	}
	mono_loader_unlock ();
}

/**
 * mono_class_get_nested_types
 * \param klass the \c MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the nested types of a class.
 * This works only if \p klass is non-generic, or a generic type definition.
 *
 * You must pass a \c gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * \returns a \c Monoclass* on each invocation, or NULL when no more are available.
 */
MonoClass*
mono_class_get_nested_types (MonoClass* klass, gpointer *iter)
{
	GList *item;

	if (!iter)
		return NULL;
	if (!klass->nested_classes_inited)
		setup_nested_types (klass);

	if (!*iter) {
		GList *nested_classes = mono_class_get_nested_classes_property (klass);
		/* start from the first */
		if (nested_classes) {
			*iter = nested_classes;
			return (MonoClass *)nested_classes->data;
		} else {
			/* no nested types */
			return NULL;
		}
	}
	item = (GList *)*iter;
	item = item->next;
	if (item) {
		*iter = item;
		return (MonoClass *)item->data;
	}
	return NULL;
}


/**
 * mono_class_is_delegate
 * \param klass the \c MonoClass to act on
 *
 * \returns TRUE if the \c MonoClass represents a \c System.Delegate.
 */
mono_bool
mono_class_is_delegate (MonoClass *klass)
{
	return klass->delegate;
}

/**
 * mono_class_implements_interface
 * \param klass The MonoClass to act on
 * \param interface The interface to check if \p klass implements.
 *
 * \returns TRUE if \p klass implements \p interface.
 */
mono_bool
mono_class_implements_interface (MonoClass* klass, MonoClass* iface)
{
	return mono_class_is_assignable_from (iface, klass);
}

/**
 * mono_field_get_name:
 * \param field the \c MonoClassField to act on
 *
 * \returns The name of the field.
 */
const char*
mono_field_get_name (MonoClassField *field)
{
	return field->name;
}

/**
 * mono_field_get_type:
 * \param field the \c MonoClassField to act on
 * \returns \c MonoType of the field.
 */
MonoType*
mono_field_get_type (MonoClassField *field)
{
	ERROR_DECL (error);
	MonoType *type = mono_field_get_type_checked (field, error);
	if (!mono_error_ok (error)) {
		mono_trace_warning (MONO_TRACE_TYPE, "Could not load field's type due to %s", mono_error_get_message (error));
		mono_error_cleanup (error);
	}
	return type;
}


/**
 * mono_field_get_type_checked:
 * \param field the \c MonoClassField to act on
 * \param error used to return any error found while retrieving \p field type
 *
 * \returns \c MonoType of the field.
 */
MonoType*
mono_field_get_type_checked (MonoClassField *field, MonoError *error)
{
	error_init (error);
	if (!field->type)
		mono_field_resolve_type (field, error);
	return field->type;
}

/**
 * mono_field_get_parent:
 * \param field the \c MonoClassField to act on
 *
 * \returns \c MonoClass where the field was defined.
 */
MonoClass*
mono_field_get_parent (MonoClassField *field)
{
	return field->parent;
}

/**
 * mono_field_get_flags;
 * \param field the \c MonoClassField to act on
 *
 * The metadata flags for a field are encoded using the
 * \c FIELD_ATTRIBUTE_* constants.  See the \c tabledefs.h file for details.
 *
 * \returns The flags for the field.
 */
guint32
mono_field_get_flags (MonoClassField *field)
{
	if (!field->type)
		return mono_field_resolve_flags (field);
	return field->type->attrs;
}

/**
 * mono_field_get_offset:
 * \param field the \c MonoClassField to act on
 *
 * \returns The field offset.
 */
guint32
mono_field_get_offset (MonoClassField *field)
{
	return field->offset;
}

static const char *
mono_field_get_rva (MonoClassField *field)
{
	guint32 rva;
	int field_index;
	MonoClass *klass = field->parent;
	MonoFieldDefaultValue *def_values;

	g_assert (field->type->attrs & FIELD_ATTRIBUTE_HAS_FIELD_RVA);

	def_values = mono_class_get_field_def_values (klass);
	if (!def_values) {
		def_values = (MonoFieldDefaultValue *)mono_class_alloc0 (klass, sizeof (MonoFieldDefaultValue) * mono_class_get_field_count (klass));

		mono_class_set_field_def_values (klass, def_values);
	}

	field_index = mono_field_get_index (field);
		
	if (!def_values [field_index].data && !image_is_dynamic (klass->image)) {
		int first_field_idx = mono_class_get_first_field_idx (klass);
		mono_metadata_field_info (field->parent->image, first_field_idx + field_index, NULL, &rva, NULL);
		if (!rva)
			g_warning ("field %s in %s should have RVA data, but hasn't", mono_field_get_name (field), field->parent->name);
		def_values [field_index].data = mono_image_rva_map (field->parent->image, rva);
	}

	return def_values [field_index].data;
}

/**
 * mono_field_get_data:
 * \param field the \c MonoClassField to act on
 *
 * \returns A pointer to the metadata constant value or to the field
 * data if it has an RVA flag.
 */
const char *
mono_field_get_data (MonoClassField *field)
{
	if (field->type->attrs & FIELD_ATTRIBUTE_HAS_DEFAULT) {
		MonoTypeEnum def_type;

		return mono_class_get_field_default_value (field, &def_type);
	} else if (field->type->attrs & FIELD_ATTRIBUTE_HAS_FIELD_RVA) {
		return mono_field_get_rva (field);
	} else {
		return NULL;
	}
}

/**
 * mono_property_get_name: 
 * \param prop the \c MonoProperty to act on
 * \returns The name of the property
 */
const char*
mono_property_get_name (MonoProperty *prop)
{
	return prop->name;
}

/**
 * mono_property_get_set_method
 * \param prop the \c MonoProperty to act on.
 * \returns The setter method of the property, a \c MonoMethod.
 */
MonoMethod*
mono_property_get_set_method (MonoProperty *prop)
{
	return prop->set;
}

/**
 * mono_property_get_get_method
 * \param prop the MonoProperty to act on.
 * \returns The getter method of the property (A \c MonoMethod)
 */
MonoMethod*
mono_property_get_get_method (MonoProperty *prop)
{
	return prop->get;
}

/**
 * mono_property_get_parent:
 * \param prop the \c MonoProperty to act on.
 * \returns The \c MonoClass where the property was defined.
 */
MonoClass*
mono_property_get_parent (MonoProperty *prop)
{
	return prop->parent;
}

/**
 * mono_property_get_flags:
 * \param prop the \c MonoProperty to act on.
 *
 * The metadata flags for a property are encoded using the
 * \c PROPERTY_ATTRIBUTE_* constants.  See the \c tabledefs.h file for details.
 *
 * \returns The flags for the property.
 */
guint32
mono_property_get_flags (MonoProperty *prop)
{
	return prop->attrs;
}

/**
 * mono_event_get_name:
 * \param event the MonoEvent to act on
 * \returns The name of the event.
 */
const char*
mono_event_get_name (MonoEvent *event)
{
	return event->name;
}

/**
 * mono_event_get_add_method:
 * \param event The \c MonoEvent to act on.
 * \returns The \c add method for the event, a \c MonoMethod.
 */
MonoMethod*
mono_event_get_add_method (MonoEvent *event)
{
	return event->add;
}

/**
 * mono_event_get_remove_method:
 * \param event The \c MonoEvent to act on.
 * \returns The \c remove method for the event, a \c MonoMethod.
 */
MonoMethod*
mono_event_get_remove_method (MonoEvent *event)
{
	return event->remove;
}

/**
 * mono_event_get_raise_method:
 * \param event The \c MonoEvent to act on.
 * \returns The \c raise method for the event, a \c MonoMethod.
 */
MonoMethod*
mono_event_get_raise_method (MonoEvent *event)
{
	return event->raise;
}

/**
 * mono_event_get_parent:
 * \param event the MonoEvent to act on.
 * \returns The \c MonoClass where the event is defined.
 */
MonoClass*
mono_event_get_parent (MonoEvent *event)
{
	return event->parent;
}

/**
 * mono_event_get_flags
 * \param event the \c MonoEvent to act on.
 *
 * The metadata flags for an event are encoded using the
 * \c EVENT_* constants.  See the \c tabledefs.h file for details.
 *
 * \returns The flags for the event.
 */
guint32
mono_event_get_flags (MonoEvent *event)
{
	return event->attrs;
}

/**
 * mono_class_get_method_from_name:
 * \param klass where to look for the method
 * \param name name of the method
 * \param param_count number of parameters. -1 for any number.
 *
 * Obtains a \c MonoMethod with a given name and number of parameters.
 * It only works if there are no multiple signatures for any given method name.
 */
MonoMethod *
mono_class_get_method_from_name (MonoClass *klass, const char *name, int param_count)
{
	return mono_class_get_method_from_name_flags (klass, name, param_count, 0);
}

MonoMethod*
mono_find_method_in_metadata (MonoClass *klass, const char *name, int param_count, int flags)
{
	MonoMethod *res = NULL;
	int i;

	/* Search directly in the metadata to avoid calling setup_methods () */
	int first_idx = mono_class_get_first_method_idx (klass);
	int mcount = mono_class_get_method_count (klass);
	for (i = 0; i < mcount; ++i) {
		ERROR_DECL (error);
		guint32 cols [MONO_METHOD_SIZE];
		MonoMethod *method;
		MonoMethodSignature *sig;

		/* first_idx points into the methodptr table */
		mono_metadata_decode_table_row (klass->image, MONO_TABLE_METHOD, first_idx + i, cols, MONO_METHOD_SIZE);

		if (!strcmp (mono_metadata_string_heap (klass->image, cols [MONO_METHOD_NAME]), name)) {
			method = mono_get_method_checked (klass->image, MONO_TOKEN_METHOD_DEF | (first_idx + i + 1), klass, NULL, error);
			if (!method) {
				mono_error_cleanup (error); /* FIXME don't swallow the error */
				continue;
			}
			if (param_count == -1) {
				res = method;
				break;
			}
			sig = mono_method_signature_checked (method, error);
			if (!sig) {
				mono_error_cleanup (error); /* FIXME don't swallow the error */
				continue;
			}
			if (sig->param_count == param_count) {
				res = method;
				break;
			}
		}
	}

	return res;
}

/**
 * mono_class_get_method_from_name_flags:
 * \param klass where to look for the method
 * \param name_space name of the method
 * \param param_count number of parameters. -1 for any number.
 * \param flags flags which must be set in the method
 *
 * Obtains a \c MonoMethod with a given name and number of parameters.
 * It only works if there are no multiple signatures for any given method name.
 */
MonoMethod *
mono_class_get_method_from_name_flags (MonoClass *klass, const char *name, int param_count, int flags)
{
	ERROR_DECL (error);
	error_init (error);

	MonoMethod * const method = mono_class_get_method_from_name_checked (klass, name, param_count, flags, error);

	mono_error_cleanup (error);
	return method;
}

/**
 * mono_class_get_method_from_name_checked:
 * \param klass where to look for the method
 * \param name_space name of the method
 * \param param_count number of parameters. -1 for any number.
 * \param flags flags which must be set in the method
 * \param error
 *
 * Obtains a \c MonoMethod with a given name and number of parameters.
 * It only works if there are no multiple signatures for any given method name.
 */
MonoMethod *
mono_class_get_method_from_name_checked (MonoClass *klass, const char *name,
	int param_count, int flags, MonoError *error)
{
	MonoMethod *res = NULL;
	int i;

	mono_class_init (klass);

	if (mono_class_is_ginst (klass) && !klass->methods) {
		res = mono_class_get_method_from_name_checked (mono_class_get_generic_class (klass)->container_class, name, param_count, flags, error);

		if (res)
			res = mono_class_inflate_generic_method_full_checked (res, klass, mono_class_get_context (klass), error);

		return res;
	}

	if (klass->methods || !MONO_CLASS_HAS_STATIC_METADATA (klass)) {
		mono_class_setup_methods (klass);
		/*
		We can't fail lookup of methods otherwise the runtime will burst in flames on all sort of places.
		See mono/tests/array_load_exception.il
		FIXME we should better report this error to the caller
		 */
		if (!klass->methods)
			return NULL;
		int mcount = mono_class_get_method_count (klass);
		for (i = 0; i < mcount; ++i) {
			MonoMethod *method = klass->methods [i];

			if (method->name[0] == name [0] && 
				!strcmp (name, method->name) &&
				(param_count == -1 || mono_method_signature (method)->param_count == param_count) &&
				((method->flags & flags) == flags)) {
				res = method;
				break;
			}
		}
	}
	else {
	    res = mono_find_method_in_metadata (klass, name, param_count, flags);
	}

	return res;
}

/**
 * mono_class_set_failure:
 * \param klass class in which the failure was detected
 * \param ex_type the kind of exception/error to be thrown (later)
 * \param ex_data exception data (specific to each type of exception/error)
 *
 * Keep a detected failure informations in the class for later processing.
 * Note that only the first failure is kept.
 *
 * LOCKING: Acquires the loader lock.
 */
gboolean
mono_class_set_failure (MonoClass *klass, MonoErrorBoxed *boxed_error)
{
	g_assert (boxed_error != NULL);

	if (mono_class_has_failure (klass))
		return FALSE;

	mono_loader_lock ();
	klass->has_failure = 1;
	mono_class_set_exception_data (klass, boxed_error);
	mono_loader_unlock ();

	return TRUE;
}

gboolean
mono_class_has_failure (const MonoClass *klass)
{
	g_assert (klass != NULL);
	return klass->has_failure != 0;
}


/**
 * mono_class_set_type_load_failure:
 * \param klass class in which the failure was detected
 * \param fmt \c printf -style error message string.
 *
 * Collect detected failure informaion in the class for later processing.
 * The error is stored as a MonoErrorBoxed as with mono_error_set_type_load_class()
 * Note that only the first failure is kept.
 *
 * LOCKING: Acquires the loader lock.
 *
 * \returns FALSE if a failure was already set on the class, or TRUE otherwise.
 */
gboolean
mono_class_set_type_load_failure (MonoClass *klass, const char * fmt, ...)
{
	ERROR_DECL_VALUE (prepare_error);
	va_list args;

	if (mono_class_has_failure (klass))
		return FALSE;
	
	error_init (&prepare_error);
	
	va_start (args, fmt);
	mono_error_vset_type_load_class (&prepare_error, klass, fmt, args);
	va_end (args);

	MonoErrorBoxed *box = mono_error_box (&prepare_error, klass->image);
	mono_error_cleanup (&prepare_error);
	return mono_class_set_failure (klass, box);
}

/**
 * mono_class_get_exception_for_failure:
 * \param klass class in which the failure was detected
 *
 * \returns a constructed MonoException than the caller can then throw
 * using mono_raise_exception - or NULL if no failure is present (or
 * doesn't result in an exception).
 */
MonoException*
mono_class_get_exception_for_failure (MonoClass *klass)
{
	if (!mono_class_has_failure (klass))
		return NULL;
	ERROR_DECL_VALUE (unboxed_error);
	error_init (&unboxed_error);
	mono_error_set_for_class_failure (&unboxed_error, klass);
	return mono_error_convert_to_exception (&unboxed_error);
}

static gboolean
is_nesting_type (MonoClass *outer_klass, MonoClass *inner_klass)
 {
	outer_klass = mono_class_get_generic_type_definition (outer_klass);
	inner_klass = mono_class_get_generic_type_definition (inner_klass);
	do {
		if (outer_klass == inner_klass)
			return TRUE;
		inner_klass = inner_klass->nested_in;
	} while (inner_klass);
	return FALSE;
}

MonoClass *
mono_class_get_generic_type_definition (MonoClass *klass)
{
	MonoGenericClass *gklass =  mono_class_try_get_generic_class (klass);
	return gklass ? gklass->container_class : klass;
}

/*
 * Check if @klass is a subtype of @parent ignoring generic instantiations.
 * 
 * Generic instantiations are ignored for all super types of @klass.
 * 
 * Visibility checks ignoring generic instantiations.  
 */
gboolean
mono_class_has_parent_and_ignore_generics (MonoClass *klass, MonoClass *parent)
{
	int i;
	klass = mono_class_get_generic_type_definition (klass);
	parent = mono_class_get_generic_type_definition (parent);
	mono_class_setup_supertypes (klass);

	for (i = 0; i < klass->idepth; ++i) {
		if (parent == mono_class_get_generic_type_definition (klass->supertypes [i]))
			return TRUE;
	}
	return FALSE;
}
/*
 * Subtype can only access parent members with family protection if the site object
 * is subclass of Subtype. For example:
 * class A { protected int x; }
 * class B : A {
 * 	void valid_access () {
 * 		B b;
 * 		b.x = 0;
 *  }
 *  void invalid_access () {
 *		A a;
 * 		a.x = 0;
 *  }
 * }
 * */
static gboolean
is_valid_family_access (MonoClass *access_klass, MonoClass *member_klass, MonoClass *context_klass)
{
	if (MONO_CLASS_IS_INTERFACE (member_klass) && !MONO_CLASS_IS_INTERFACE (access_klass)) {
		/* Can happen with default interface methods */
		if (!mono_class_implements_interface (access_klass, member_klass))
			return FALSE;
	} else {
		if (!mono_class_has_parent_and_ignore_generics (access_klass, member_klass))
			return FALSE;
	}

	if (context_klass == NULL)
		return TRUE;
	/*if access_klass is not member_klass context_klass must be type compat*/
	if (access_klass != member_klass && !mono_class_has_parent_and_ignore_generics (context_klass, access_klass))
		return FALSE;
	return TRUE;
}

static gboolean
can_access_internals (MonoAssembly *accessing, MonoAssembly* accessed)
{
	GSList *tmp;
	if (accessing == accessed)
		return TRUE;
	if (!accessed || !accessing)
		return FALSE;

	/* extra safety under CoreCLR - the runtime does not verify the strongname signatures
	 * anywhere so untrusted friends are not safe to access platform's code internals */
	if (mono_security_core_clr_enabled ()) {
		if (!mono_security_core_clr_can_access_internals (accessing->image, accessed->image))
			return FALSE;
	}

	mono_assembly_load_friends (accessed);
	for (tmp = accessed->friend_assembly_names; tmp; tmp = tmp->next) {
		MonoAssemblyName *friend_ = (MonoAssemblyName *)tmp->data;
		/* Be conservative with checks */
		if (!friend_->name)
			continue;
		if (g_ascii_strcasecmp (accessing->aname.name, friend_->name))
			continue;
		if (friend_->public_key_token [0]) {
			if (!accessing->aname.public_key_token [0])
				continue;
			if (!mono_public_tokens_are_equal (friend_->public_key_token, accessing->aname.public_key_token))
				continue;
		}
		return TRUE;
	}
	return FALSE;
}

/*
 * If klass is a generic type or if it is derived from a generic type, return the
 * MonoClass of the generic definition
 * Returns NULL if not found
 */
static MonoClass*
get_generic_definition_class (MonoClass *klass)
{
	while (klass) {
		MonoGenericClass *gklass = mono_class_try_get_generic_class (klass);
		if (gklass && gklass->container_class)
			return gklass->container_class;
		klass = klass->parent;
	}
	return NULL;
}

static gboolean
can_access_instantiation (MonoClass *access_klass, MonoGenericInst *ginst)
{
	int i;
	for (i = 0; i < ginst->type_argc; ++i) {
		MonoType *type = ginst->type_argv[i];
		switch (type->type) {
		case MONO_TYPE_SZARRAY:
			if (!can_access_type (access_klass, type->data.klass))
				return FALSE;
			break;
		case MONO_TYPE_ARRAY:
			if (!can_access_type (access_klass, type->data.array->eklass))
				return FALSE;
			break;
		case MONO_TYPE_PTR:
			if (!can_access_type (access_klass, mono_class_from_mono_type (type->data.type)))
				return FALSE;
			break;
		case MONO_TYPE_CLASS:
		case MONO_TYPE_VALUETYPE:
		case MONO_TYPE_GENERICINST:
			if (!can_access_type (access_klass, mono_class_from_mono_type (type)))
				return FALSE;
		default:
			break;
		}
	}
	return TRUE;
}

static gboolean
can_access_type (MonoClass *access_klass, MonoClass *member_klass)
{
	int access_level;

	if (access_klass == member_klass)
		return TRUE;

	if (access_klass->image->assembly && access_klass->image->assembly->corlib_internal)
		return TRUE;

	if (access_klass->element_class && !access_klass->enumtype)
		access_klass = access_klass->element_class;

	if (member_klass->element_class && !member_klass->enumtype)
		member_klass = member_klass->element_class;

	access_level = mono_class_get_flags (member_klass) & TYPE_ATTRIBUTE_VISIBILITY_MASK;

	if (member_klass->byval_arg.type == MONO_TYPE_VAR || member_klass->byval_arg.type == MONO_TYPE_MVAR)
		return TRUE;

	if (mono_class_is_ginst (member_klass) && !can_access_instantiation (access_klass, mono_class_get_generic_class (member_klass)->context.class_inst))
		return FALSE;

	if (is_nesting_type (access_klass, member_klass) || (access_klass->nested_in && is_nesting_type (access_klass->nested_in, member_klass)))
		return TRUE;

	if (member_klass->nested_in && !can_access_type (access_klass, member_klass->nested_in))
		return FALSE;

	/*Non nested type with nested visibility. We just fail it.*/
	if (access_level >= TYPE_ATTRIBUTE_NESTED_PRIVATE && access_level <= TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM && member_klass->nested_in == NULL)
		return FALSE;

	switch (access_level) {
	case TYPE_ATTRIBUTE_NOT_PUBLIC:
		return can_access_internals (access_klass->image->assembly, member_klass->image->assembly);

	case TYPE_ATTRIBUTE_PUBLIC:
		return TRUE;

	case TYPE_ATTRIBUTE_NESTED_PUBLIC:
		return TRUE;

	case TYPE_ATTRIBUTE_NESTED_PRIVATE:
		return is_nesting_type (member_klass, access_klass);

	case TYPE_ATTRIBUTE_NESTED_FAMILY:
		return mono_class_has_parent_and_ignore_generics (access_klass, member_klass->nested_in); 

	case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
		return can_access_internals (access_klass->image->assembly, member_klass->image->assembly);

	case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
		return can_access_internals (access_klass->image->assembly, member_klass->nested_in->image->assembly) &&
			mono_class_has_parent_and_ignore_generics (access_klass, member_klass->nested_in);

	case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
		return can_access_internals (access_klass->image->assembly, member_klass->nested_in->image->assembly) ||
			mono_class_has_parent_and_ignore_generics (access_klass, member_klass->nested_in);
	}
	return FALSE;
}

/* FIXME: check visibility of type, too */
static gboolean
can_access_member (MonoClass *access_klass, MonoClass *member_klass, MonoClass* context_klass, int access_level)
{
	MonoClass *member_generic_def;
	if (access_klass->image->assembly && access_klass->image->assembly->corlib_internal)
		return TRUE;

	MonoGenericClass *access_gklass = mono_class_try_get_generic_class (access_klass);
	if (((access_gklass && access_gklass->container_class) ||
					mono_class_is_gtd (access_klass)) && 
			(member_generic_def = get_generic_definition_class (member_klass))) {
		MonoClass *access_container;

		if (mono_class_is_gtd (access_klass))
			access_container = access_klass;
		else
			access_container = access_gklass->container_class;

		if (can_access_member (access_container, member_generic_def, context_klass, access_level))
			return TRUE;
	}

	/* Partition I 8.5.3.2 */
	/* the access level values are the same for fields and methods */
	switch (access_level) {
	case FIELD_ATTRIBUTE_COMPILER_CONTROLLED:
		/* same compilation unit */
		return access_klass->image == member_klass->image;
	case FIELD_ATTRIBUTE_PRIVATE:
		return access_klass == member_klass;
	case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
		if (is_valid_family_access (access_klass, member_klass, context_klass) &&
		    can_access_internals (access_klass->image->assembly, member_klass->image->assembly))
			return TRUE;
		return FALSE;
	case FIELD_ATTRIBUTE_ASSEMBLY:
		return can_access_internals (access_klass->image->assembly, member_klass->image->assembly);
	case FIELD_ATTRIBUTE_FAMILY:
		if (is_valid_family_access (access_klass, member_klass, context_klass))
			return TRUE;
		return FALSE;
	case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
		if (is_valid_family_access (access_klass, member_klass, context_klass))
			return TRUE;
		return can_access_internals (access_klass->image->assembly, member_klass->image->assembly);
	case FIELD_ATTRIBUTE_PUBLIC:
		return TRUE;
	}
	return FALSE;
}

/**
 * mono_method_can_access_field:
 * \param method Method that will attempt to access the field
 * \param field the field to access
 *
 * Used to determine if a method is allowed to access the specified field.
 *
 * \returns TRUE if the given \p method is allowed to access the \p field while following
 * the accessibility rules of the CLI.
 */
gboolean
mono_method_can_access_field (MonoMethod *method, MonoClassField *field)
{
	/* FIXME: check all overlapping fields */
	int can = can_access_member (method->klass, field->parent, NULL, mono_field_get_type (field)->attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK);
	if (!can) {
		MonoClass *nested = method->klass->nested_in;
		while (nested) {
			can = can_access_member (nested, field->parent, NULL, mono_field_get_type (field)->attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK);
			if (can)
				return TRUE;
			nested = nested->nested_in;
		}
	}
	return can;
}

static MonoMethod*
mono_method_get_method_definition (MonoMethod *method)
{
	while (method->is_inflated)
		method = ((MonoMethodInflated*)method)->declaring;
	return method;
}

/**
 * mono_method_can_access_method:
 * \param method Method that will attempt to access the other method
 * \param called the method that we want to probe for accessibility.
 *
 * Used to determine if the \p method is allowed to access the specified \p called method.
 *
 * \returns TRUE if the given \p method is allowed to invoke the \p called while following
 * the accessibility rules of the CLI.
 */
gboolean
mono_method_can_access_method (MonoMethod *method, MonoMethod *called)
{
	method = mono_method_get_method_definition (method);
	called = mono_method_get_method_definition (called);
	return mono_method_can_access_method_full (method, called, NULL);
}

/*
 * mono_method_can_access_method_full:
 * @method: The caller method 
 * @called: The called method 
 * @context_klass: The static type on stack of the owner @called object used
 * 
 * This function must be used with instance calls, as they have more strict family accessibility.
 * It can be used with static methods, but context_klass should be NULL.
 * 
 * Returns: TRUE if caller have proper visibility and acessibility to @called
 */
gboolean
mono_method_can_access_method_full (MonoMethod *method, MonoMethod *called, MonoClass *context_klass)
{
	/* Wrappers are except from access checks */
	if (method->wrapper_type != MONO_WRAPPER_NONE || called->wrapper_type != MONO_WRAPPER_NONE)
		return TRUE;

	MonoClass *access_class = method->klass;
	MonoClass *member_class = called->klass;
	int can = can_access_member (access_class, member_class, context_klass, called->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK);
	if (!can) {
		MonoClass *nested = access_class->nested_in;
		while (nested) {
			can = can_access_member (nested, member_class, context_klass, called->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK);
			if (can)
				break;
			nested = nested->nested_in;
		}
	}

	if (!can)
		return FALSE;

	can = can_access_type (access_class, member_class);
	if (!can) {
		MonoClass *nested = access_class->nested_in;
		while (nested) {
			can = can_access_type (nested, member_class);
			if (can)
				break;
			nested = nested->nested_in;
		}
	}

	if (!can)
		return FALSE;

	if (called->is_inflated) {
		MonoMethodInflated * infl = (MonoMethodInflated*)called;
		if (infl->context.method_inst && !can_access_instantiation (access_class, infl->context.method_inst))
			return FALSE;
	}
		
	return TRUE;
}


/*
 * mono_method_can_access_field_full:
 * @method: The caller method 
 * @field: The accessed field
 * @context_klass: The static type on stack of the owner @field object used
 * 
 * This function must be used with instance fields, as they have more strict family accessibility.
 * It can be used with static fields, but context_klass should be NULL.
 * 
 * Returns: TRUE if caller have proper visibility and acessibility to @field
 */
gboolean
mono_method_can_access_field_full (MonoMethod *method, MonoClassField *field, MonoClass *context_klass)
{
	MonoClass *access_class = method->klass;
	MonoClass *member_class = field->parent;
	/* FIXME: check all overlapping fields */
	int can = can_access_member (access_class, member_class, context_klass, field->type->attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK);
	if (!can) {
		MonoClass *nested = access_class->nested_in;
		while (nested) {
			can = can_access_member (nested, member_class, context_klass, field->type->attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK);
			if (can)
				break;
			nested = nested->nested_in;
		}
	}

	if (!can)
		return FALSE;

	can = can_access_type (access_class, member_class);
	if (!can) {
		MonoClass *nested = access_class->nested_in;
		while (nested) {
			can = can_access_type (nested, member_class);
			if (can)
				break;
			nested = nested->nested_in;
		}
	}

	if (!can)
		return FALSE;
	return TRUE;
}

/*
 * mono_class_can_access_class:
 * @source_class: The source class 
 * @target_class: The accessed class
 * 
 * This function returns is @target_class is visible to @source_class
 * 
 * Returns: TRUE if source have proper visibility and acessibility to target
 */
gboolean
mono_class_can_access_class (MonoClass *source_class, MonoClass *target_class)
{
	return can_access_type (source_class, target_class);
}

/**
 * mono_type_is_valid_enum_basetype:
 * \param type The MonoType to check
 * \returns TRUE if the type can be used as the basetype of an enum
 */
gboolean mono_type_is_valid_enum_basetype (MonoType * type) {
	switch (type->type) {
	case MONO_TYPE_I1:
	case MONO_TYPE_U1:
	case MONO_TYPE_BOOLEAN:
	case MONO_TYPE_I2:
	case MONO_TYPE_U2:
	case MONO_TYPE_CHAR:
	case MONO_TYPE_I4:
	case MONO_TYPE_U4:
	case MONO_TYPE_I8:
	case MONO_TYPE_U8:
	case MONO_TYPE_I:
	case MONO_TYPE_U:
		return TRUE;
	default:
		return FALSE;
	}
}

/**
 * mono_class_is_valid_enum:
 * \param klass An enum class to be validated
 *
 * This method verify the required properties an enum should have.
 *
 * FIXME: TypeBuilder enums are allowed to implement interfaces, but since they cannot have methods, only empty interfaces are possible
 * FIXME: enum types are not allowed to have a cctor, but mono_reflection_create_runtime_class sets has_cctor to 1 for all types
 * FIXME: TypeBuilder enums can have any kind of static fields, but the spec is very explicit about that (P II 14.3)
 *
 * \returns TRUE if the informed enum class is valid 
 */
gboolean
mono_class_is_valid_enum (MonoClass *klass)
{
	MonoClassField * field;
	gpointer iter = NULL;
	gboolean found_base_field = FALSE;

	g_assert (klass->enumtype);
	/* we cannot test against mono_defaults.enum_class, or mcs won't be able to compile the System namespace*/
	if (!klass->parent || strcmp (klass->parent->name, "Enum") || strcmp (klass->parent->name_space, "System") ) {
		return FALSE;
	}

	if (!mono_class_is_auto_layout (klass))
		return FALSE;

	while ((field = mono_class_get_fields (klass, &iter))) {
		if (!(field->type->attrs & FIELD_ATTRIBUTE_STATIC)) {
			if (found_base_field)
				return FALSE;
			found_base_field = TRUE;
			if (!mono_type_is_valid_enum_basetype (field->type))
				return FALSE;
		}
	}

	if (!found_base_field)
		return FALSE;

	if (mono_class_get_method_count (klass) > 0)
		return FALSE;

	return TRUE;
}

gboolean
mono_generic_class_is_generic_type_definition (MonoGenericClass *gklass)
{
	return gklass->context.class_inst == mono_class_get_generic_container (gklass->container_class)->context.class_inst;
}

void
mono_field_resolve_type (MonoClassField *field, MonoError *error)
{
	MonoClass *klass = field->parent;
	MonoImage *image = klass->image;
	MonoClass *gtd = mono_class_is_ginst (klass) ? mono_class_get_generic_type_definition (klass) : NULL;
	MonoType *ftype;
	int field_idx = field - klass->fields;

	error_init (error);

	if (gtd) {
		MonoClassField *gfield = &gtd->fields [field_idx];
		MonoType *gtype = mono_field_get_type_checked (gfield, error);
		if (!mono_error_ok (error)) {
			char *full_name = mono_type_get_full_name (gtd);
			mono_class_set_type_load_failure (klass, "Could not load generic type of field '%s:%s' (%d) due to: %s", full_name, gfield->name, field_idx, mono_error_get_message (error));
			g_free (full_name);
		}

		ftype = mono_class_inflate_generic_type_no_copy (image, gtype, mono_class_get_context (klass), error);
		if (!mono_error_ok (error)) {
			char *full_name = mono_type_get_full_name (klass);
			mono_class_set_type_load_failure (klass, "Could not load instantiated type of field '%s:%s' (%d) due to: %s", full_name, field->name, field_idx, mono_error_get_message (error));
			g_free (full_name);
		}
	} else {
		const char *sig;
		guint32 cols [MONO_FIELD_SIZE];
		MonoGenericContainer *container = NULL;
		int idx = mono_class_get_first_field_idx (klass) + field_idx;

		/*FIXME, in theory we do not lazy load SRE fields*/
		g_assert (!image_is_dynamic (image));

		if (mono_class_is_gtd (klass)) {
			container = mono_class_get_generic_container (klass);
		} else if (gtd) {
			container = mono_class_get_generic_container (gtd);
			g_assert (container);
		}

		/* first_field_idx and idx points into the fieldptr table */
		mono_metadata_decode_table_row (image, MONO_TABLE_FIELD, idx, cols, MONO_FIELD_SIZE);

		if (!mono_verifier_verify_field_signature (image, cols [MONO_FIELD_SIGNATURE], NULL)) {
			char *full_name = mono_type_get_full_name (klass);
			mono_error_set_type_load_class (error, klass, "Could not verify field '%s:%s' signature", full_name, field->name);;
			mono_class_set_type_load_failure (klass, "%s", mono_error_get_message (error));
			g_free (full_name);
			return;
		}

		sig = mono_metadata_blob_heap (image, cols [MONO_FIELD_SIGNATURE]);

		mono_metadata_decode_value (sig, &sig);
		/* FIELD signature == 0x06 */
		g_assert (*sig == 0x06);

		ftype = mono_metadata_parse_type_checked (image, container, cols [MONO_FIELD_FLAGS], FALSE, sig + 1, &sig, error);
		if (!ftype) {
			char *full_name = mono_type_get_full_name (klass);
			mono_class_set_type_load_failure (klass, "Could not load type of field '%s:%s' (%d) due to: %s", full_name, field->name, field_idx, mono_error_get_message (error));
			g_free (full_name);
		}
	}
	mono_memory_barrier ();
	field->type = ftype;
}

static guint32
mono_field_resolve_flags (MonoClassField *field)
{
	MonoClass *klass = field->parent;
	MonoImage *image = klass->image;
	MonoClass *gtd = mono_class_is_ginst (klass) ? mono_class_get_generic_type_definition (klass) : NULL;
	int field_idx = field - klass->fields;

	if (gtd) {
		MonoClassField *gfield = &gtd->fields [field_idx];
		return mono_field_get_flags (gfield);
	} else {
		int idx = mono_class_get_first_field_idx (klass) + field_idx;

		/*FIXME, in theory we do not lazy load SRE fields*/
		g_assert (!image_is_dynamic (image));

		return mono_metadata_decode_table_row_col (image, MONO_TABLE_FIELD, idx, MONO_FIELD_FLAGS);
	}
}

/**
 * mono_class_get_fields_lazy:
 * \param klass the MonoClass to act on
 *
 * This routine is an iterator routine for retrieving the fields in a class.
 * Only minimal information about fields are loaded. Accessors must be used
 * for all MonoClassField returned.
 *
 * You must pass a gpointer that points to zero and is treated as an opaque handle to
 * iterate over all of the elements.  When no more values are
 * available, the return value is NULL.
 *
 * \returns a \c MonoClassField* on each iteration, or NULL when no more fields are available.
 */
MonoClassField*
mono_class_get_fields_lazy (MonoClass* klass, gpointer *iter)
{
	MonoClassField* field;
	if (!iter)
		return NULL;
	if (!*iter) {
		mono_class_setup_basic_field_info (klass);
		if (!klass->fields)
			return NULL;
		/* start from the first */
		if (mono_class_get_field_count (klass)) {
			*iter = &klass->fields [0];
			return (MonoClassField *)*iter;
		} else {
			/* no fields */
			return NULL;
		}
	}
	field = (MonoClassField *)*iter;
	field++;
	if (field < &klass->fields [mono_class_get_field_count (klass)]) {
		*iter = field;
		return (MonoClassField *)*iter;
	}
	return NULL;
}

char*
mono_class_full_name (MonoClass *klass)
{
	return mono_type_full_name (&klass->byval_arg);
}

/* Declare all shared lazy type lookup functions */
GENERATE_TRY_GET_CLASS_WITH_CACHE (safehandle, "System.Runtime.InteropServices", "SafeHandle")

/**
 * mono_method_get_base_method:
 * \param method a method
 * \param definition if true, get the definition
 * \param error set on failure
 *
 * Given a virtual method associated with a subclass, return the corresponding
 * method from an ancestor.  If \p definition is FALSE, returns the method in the
 * superclass of the given method.  If \p definition is TRUE, return the method
 * in the ancestor class where it was first declared.  The type arguments will
 * be inflated in the ancestor classes.  If the method is not associated with a
 * class, or isn't virtual, returns the method itself.  On failure returns NULL
 * and sets \p error.
 */
MonoMethod*
mono_method_get_base_method (MonoMethod *method, gboolean definition, MonoError *error)
{
	MonoClass *klass, *parent;
	MonoGenericContext *generic_inst = NULL;
	MonoMethod *result = NULL;
	int slot;

	if (method->klass == NULL)
		return method;

	if (!(method->flags & METHOD_ATTRIBUTE_VIRTUAL) ||
	    MONO_CLASS_IS_INTERFACE (method->klass) ||
	    method->flags & METHOD_ATTRIBUTE_NEW_SLOT)
		return method;

	slot = mono_method_get_vtable_slot (method);
	if (slot == -1)
		return method;

	klass = method->klass;
	if (mono_class_is_ginst (klass)) {
		generic_inst = mono_class_get_context (klass);
		klass = mono_class_get_generic_class (klass)->container_class;
	}

retry:
	if (definition) {
		/* At the end of the loop, klass points to the eldest class that has this virtual function slot. */
		for (parent = klass->parent; parent != NULL; parent = parent->parent) {
			/* on entry, klass is either a plain old non-generic class and generic_inst == NULL
			   or klass is the generic container class and generic_inst is the instantiation.

			   when we go to the parent, if the parent is an open constructed type, we need to
			   replace the type parameters by the definitions from the generic_inst, and then take it
			   apart again into the klass and the generic_inst.

			   For cases like this:
			   class C<T> : B<T, int> {
			       public override void Foo () { ... }
			   }
			   class B<U,V> : A<HashMap<U,V>> {
			       public override void Foo () { ... }
			   }
			   class A<X> {
			       public virtual void Foo () { ... }
			   }

			   if at each iteration the parent isn't open, we can skip inflating it.  if at some
			   iteration the parent isn't generic (after possible inflation), we set generic_inst to
			   NULL;
			*/
			MonoGenericContext *parent_inst = NULL;
			if (mono_class_is_open_constructed_type (mono_class_get_type (parent))) {
				parent = mono_class_inflate_generic_class_checked (parent, generic_inst, error);
				return_val_if_nok  (error, NULL);
			}
			if (mono_class_is_ginst (parent)) {
				parent_inst = mono_class_get_context (parent);
				parent = mono_class_get_generic_class (parent)->container_class;
			}

			mono_class_setup_vtable (parent);
			if (parent->vtable_size <= slot)
				break;
			klass = parent;
			generic_inst = parent_inst;
		}
	} else {
		klass = klass->parent;
		if (!klass)
			return method;
		if (mono_class_is_open_constructed_type (mono_class_get_type (klass))) {
			klass = mono_class_inflate_generic_class_checked (klass, generic_inst, error);
			return_val_if_nok (error, NULL);

			generic_inst = NULL;
		}
		if (mono_class_is_ginst (klass)) {
			generic_inst = mono_class_get_context (klass);
			klass = mono_class_get_generic_class (klass)->container_class;
		}

	}

	if (generic_inst) {
		klass = mono_class_inflate_generic_class_checked (klass, generic_inst, error);
		return_val_if_nok (error, NULL);
	}

	if (klass == method->klass)
		return method;

	/*This is possible if definition == FALSE.
	 * Do it here to be really sure we don't read invalid memory.
	 */
	if (slot >= klass->vtable_size)
		return method;

	mono_class_setup_vtable (klass);

	result = klass->vtable [slot];
	if (result == NULL) {
		/* It is an abstract method */
		gboolean found = FALSE;
		gpointer iter = NULL;
		while ((result = mono_class_get_methods (klass, &iter))) {
			if (result->slot == slot) {
				found = TRUE;
				break;
			}
		}
		/* found might be FALSE if we looked in an abstract class
		 * that doesn't override an abstract method of its
		 * parent: 
		 *   abstract class Base {
		 *     public abstract void Foo ();
		 *   }
		 *   abstract class Derived : Base { }
		 *   class Child : Derived {
		 *     public override void Foo () { }
		 *  }
		 *
		 *  if m was Child.Foo and we ask for the base method,
		 *  then we get here with klass == Derived and found == FALSE
		 */
		/* but it shouldn't be the case that if we're looking
		 * for the definition and didn't find a result; the
		 * loop above should've taken us as far as we could
		 * go! */
		g_assert (!(definition && !found));
		if (!found)
			goto retry;
	}

	g_assert (result != NULL);
	return result;
}

