
/*
 * These return allocated strings
 */
char *get_typedef             (MonoImage *m, int idx);
char *get_module              (MonoImage *m, int idx);
char *get_moduleref           (MonoImage *m, int idx);
char *get_assemblyref         (MonoImage *m, int idx);
char *get_typeref             (MonoImage *m, int idx);
char *get_typedef_or_ref      (MonoImage *m, guint32 dor_token, MonoGenericContext *context);
char *dis_stringify_object_with_class (MonoImage *m, MonoClass *c, gboolean p, gboolean d);
char *get_type_or_methdef     (MonoImage *m, guint32 dor_token);
char *get_field_signature     (MonoImage *m, guint32 blob_signature, MonoGenericContext *context);
char *get_fieldref_signature  (MonoImage *m, int idx, MonoGenericContext *contxt);
char *decode_literal          (MonoImage *m, guint32 token);
char *get_field               (MonoImage *m, guint32 token, MonoGenericContext *context);
char *param_flags             (guint32 f);
char *field_flags             (guint32 f);
char *get_methodref_signature (MonoImage *m, guint32 blob_signature, const char *fancy,
			       MonoGenericContext *context);
char *get_methodspec          (MonoImage *m, int idx, guint32 token, const char *fancy,
			       MonoGenericContext *context);
char *get_constant            (MonoImage *m, MonoTypeEnum t, guint32 blob_index);
char *get_token               (MonoImage *m, guint32 token, MonoGenericContext *context);
char *get_token_type          (MonoImage *m, guint32 token, MonoGenericContext *context);
char *get_typespec            (MonoImage *m, guint32 blob_idx, MonoGenericContext *context);
char *get_methoddef           (MonoImage *m, guint32 idx);
char *get_method              (MonoImage *m, guint32 token, MonoGenericContext *context);
char *get_method_type_param   (MonoImage *m, guint32 blob_signature, MonoGenericContext *context);
char *get_guid                (MonoImage *m, guint32 guid_index);
char *get_marshal_info        (MonoImage *m, const char *blob);
char *get_generic_param       (MonoImage *m, MonoGenericContainer *container);
char *get_escaped_name        (const char *name);
char *get_method_override     (MonoImage *m, guint32 token, MonoGenericContext *context);

GList *dis_get_custom_attrs   (MonoImage *m, guint32 token);

char *dis_stringify_type      (MonoImage *m, MonoType *type, gboolean is_def);
char *dis_stringify_token     (MonoImage *m, guint32 token);
char *dis_stringify_array     (MonoImage *m, MonoArrayType *array, gboolean is_def);
char *dis_stringify_modifiers (MonoImage *m, int n, MonoCustomMod *mod);
char *dis_stringify_param     (MonoImage *m, MonoType *param);
char *dis_stringify_method_signature (MonoImage *m, MonoMethodSignature *method, int methoddef_row,
				      MonoGenericContext *context, gboolean fully_qualified);
char *dis_stringify_function_ptr (MonoImage *m, MonoMethodSignature *method);

guint32 method_dor_to_token (guint32 idx);

/*
 * These functions are used during the decoding of streams in the
 * metadata heaps (a simple parsing).
 *
 * They return the `next' location to continue parsing from (ptr is
 * the starting location).
 *
 * Results are returning in the pointer argument.
 */
const char *get_encoded_typedef_or_ref (MonoImage *m, const char *ptr,
					char **result);
const char *get_encoded_value          (const char *_ptr,
					guint32 *len);
const char *get_custom_mod             (MonoImage *m, const char *ptr,
					char **return_value);
const char *get_type                   (MonoImage *m, const char *ptr,
					char **result, MonoGenericContext *context);
const char *get_ret_type               (MonoImage *m, const char *ptr,
					char **ret_type, MonoGenericContext *context);
const char *get_param                  (MonoImage *m, const char *ptr,
					char **retval, MonoGenericContext *context);
const char *get_blob_encoded_size      (const char *ptr, int *size);

MonoTypeEnum get_field_literal_type (MonoImage *m, guint32 blob_signature);

/**
 * This is called to initialize the table containing keyword names
 */
void init_key_table (void);

extern gboolean show_method_tokens;
extern gboolean show_tokens;
