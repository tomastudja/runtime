#ifndef _MONO_CLI_OBJECT_H_
#define _MONO_CLI_OBJECT_H_

#include <mono/metadata/class.h>
#include <mono/metadata/threads-types.h>

#if 1
#ifdef __GNUC__
#define mono_assert(expr) 	           G_STMT_START{		  \
     if (!(expr))							  \
       {								  \
              	MonoException *ex;                                        \
                char *msg = g_strdup_printf ("file %s: line %d (%s): "    \
                "assertion failed: (%s)", __FILE__, __LINE__,             \
                __PRETTY_FUNCTION__, #expr);				  \
		ex = mono_get_exception_execution_engine (msg);           \
		g_free (msg);                                             \
                mono_raise_exception (ex);                                \
       };				}G_STMT_END

#define mono_assert_not_reached() 	          G_STMT_START{		  \
     MonoException *ex;                                                   \
     char *msg = g_strdup_printf ("file %s: line %d (%s): "               \
     "should not be reached", __FILE__, __LINE__, __PRETTY_FUNCTION__);	  \
     ex = mono_get_exception_execution_engine (msg);                      \
     g_free (msg);                                                        \
     mono_raise_exception (ex);                                           \
}G_STMT_END
#else /* not GNUC */
#define mono_assert(expr) 	           G_STMT_START{		  \
     if (!(expr))							  \
       {								  \
              	MonoException *ex;                                        \
                char *msg = g_strdup_printf ("file %s: line %d: "         \
                "assertion failed: (%s)", __FILE__, __LINE__,             \
                #expr);							  \
		ex = mono_get_exception_execution_engine (msg);           \
		g_free (msg);                                             \
                mono_raise_exception (ex);                                \
       };				}G_STMT_END

#define mono_assert_not_reached() 	          G_STMT_START{		  \
     MonoException *ex;                                                   \
     char *msg = g_strdup_printf ("file %s: line %d): "                   \
     "should not be reached", __FILE__, __LINE__);			  \
     ex = mono_get_exception_execution_engine (msg);                      \
     g_free (msg);                                                        \
     mono_raise_exception (ex);                                           \
}G_STMT_END
#endif
#else
#define mono_assert(expr) g_assert(expr)
#define mono_assert_not_reached() g_assert_not_reached() 
#endif

#define MONO_CHECK_ARG(arg, expr)       	G_STMT_START{		  \
     if (!(expr))							  \
       {								  \
              	MonoException *ex;                                        \
                char *msg = g_strdup_printf ("assertion `%s' failed",     \
		#expr);							  \
                if (arg) {} /* check if the name exists */                \
		ex = mono_get_exception_argument (#arg, msg);             \
		g_free (msg);                                             \
                mono_raise_exception (ex);                                \
       };				}G_STMT_END

#define MONO_CHECK_ARG_NULL(arg) 	    G_STMT_START{		  \
     if (arg == NULL)							  \
       {								  \
              	MonoException *ex;                                        \
                if (arg) {} /* check if the name exists */                \
		ex = mono_get_exception_argument_null (#arg);             \
                mono_raise_exception (ex);                                \
       };				}G_STMT_END

typedef guchar MonoBoolean;

typedef struct _MonoReflectionMethod MonoReflectionMethod;
typedef struct _MonoDelegate MonoDelegate;

typedef struct {
	MonoVTable *vtable;
	MonoThreadsSync *synchronisation;
} MonoObject;

typedef struct {
	MonoObject obj;
	MonoObject *identity;
} MonoMarshalByRefObject;

typedef struct {
	guint32 length;
	guint32 lower_bound;
} MonoArrayBounds;

typedef struct {
	MonoObject obj;
	/* bounds is NULL for szarrays */
	MonoArrayBounds *bounds;
	/* total number of elements of the array */
	guint32 max_length; 
	/* we use double to ensure proper alignment on platforms that need it */
	double vector [MONO_ZERO_LEN_ARRAY];
} MonoArray;

typedef struct {
	MonoObject object;
	gint32 length;
	gunichar2 chars [MONO_ZERO_LEN_ARRAY];
} MonoString;

typedef struct {
	MonoObject object;
	gint32 length;
	MonoString *str;
} MonoStringBuilder;

typedef struct {
	MonoObject object;
	MonoType  *type;
} MonoReflectionType;

typedef struct {
	MonoType *type;
	gpointer  value;
	MonoClass *klass;
} MonoTypedRef;

typedef struct {
	gpointer args;
} MonoArgumentHandle;

typedef struct {
	MonoMethodSignature *sig;
	gpointer args;
	gint32 next_arg;
	gint32 num_args;
} MonoArgIterator;

typedef struct {
	MonoObject object;
	MonoArray  *trace_ips;
	MonoObject *inner_ex;
	MonoString *message;
	MonoString *help_link;
	MonoString *class_name;
	MonoString *stack_trace;
	MonoString *remote_stack_trace;
	gint32      remote_stack_index;
	gint32      hresult;
	MonoString *source;
} MonoException;

typedef struct {
	MonoException base;
} MonoSystemException;

typedef struct {
	MonoSystemException base;
	MonoString *param_name;
} MonoArgumentException;

typedef struct {
	MonoSystemException base;
	MonoString *msg;
	MonoString *type_name;
} MonoTypeLoadException;

typedef struct {
	MonoObject   object;
	MonoObject  *async_state;
	MonoObject  *handle;
	MonoObject  *async_delegate;
	gpointer     data;
	MonoBoolean  sync_completed;
	MonoBoolean  completed;
	MonoBoolean  endinvoke_called;
	MonoObject  *async_callback;
} MonoAsyncResult;

typedef struct {
	MonoMarshalByRefObject object;
	gpointer     handle;
	MonoBoolean  disposed;
} MonoWaitHandle;

typedef struct {
	MonoObject  object;
	MonoReflectionType *class_to_proxy;	
	MonoObject *context;
	MonoObject *unwrapped_server;
} MonoRealProxy;

typedef struct {
	MonoObject     object;
	MonoRealProxy *rp;	
	MonoClass     *klass; 
	MonoBoolean    custom_type_info;
} MonoTransparentProxy;

/* This is a copy of System.Runtime.Remoting.Messaging.CallType */
typedef enum {
	CallType_Sync = 0,
	CallType_BeginInvoke = 1,
	CallType_EndInvoke = 2,
	CallType_OneWay = 3
} MonoCallType;

typedef struct {
	MonoObject obj;
	MonoReflectionMethod *method;
	MonoArray  *args;		
	MonoArray  *names;		
	MonoArray  *arg_types;	
	MonoObject *ctx;
	MonoObject *rval;
	MonoObject *exc;
	MonoAsyncResult *async_result;
	guint32     call_type;
} MonoMethodMessage;

typedef struct {
	MonoObject obj;
	gint32 il_offset;
	gint32 native_offset;
	MonoReflectionMethod *method;
	MonoString *filename;
	gint32 line;
	gint32 column;
} MonoStackFrame;

typedef struct {
	MonoObject  obj;
	HANDLE      handle;
	MonoObject *culture_info;
	MonoObject *ui_culture_info;
	MonoBoolean threadpool_thread;
	guint32     state;
	MonoException *abort_exc;
	MonoObject *abort_state;
	guint32 tid;
	HANDLE      start_notify;
	gpointer stack_ptr;
	gpointer *static_data;
	gpointer jit_data;
	gpointer lock_data;
	GSList *appdomain_refs;
} MonoThread;

typedef struct {
	MonoString *name;
	MonoReflectionType *type;
	MonoObject *value;
} MonoSerializationEntry;

typedef struct {
	guint32 state;
	MonoObject *additional;
} MonoStreamingContext;

typedef struct {
	MonoObject obj;
	MonoBoolean readOnly;
	MonoString *AMDesignator;
	MonoString *PMDesignator;
	MonoString *DateSeparator;
	MonoString *TimeSeparator;
	MonoString *ShortDatePattern;
	MonoString *LongDatePattern;
	MonoString *ShortTimePattern;
	MonoString *LongTimePattern;
	MonoString *MonthDayPattern;
	MonoString *YearMonthPattern;
	MonoString *FullDateTimePattern;
	MonoString *RFC1123Pattern;
	MonoString *SortableDateTimePattern;
	MonoString *UniversalSortableDateTimePattern;
	guint32 FirstDayOfWeek;
	MonoObject *Calendar;
	guint32 CalendarWeekRule;
	MonoArray *AbbreviatedDayNames;
	MonoArray *DayNames;
	MonoArray *MonthNames;
	MonoArray *AbbreviatedMonthNames;
} MonoDateTimeFormatInfo;

typedef struct 
{
	MonoObject obj;
	MonoBoolean readOnly;
	MonoString *decimalFormats;
	MonoString *currencyFormats;
	MonoString *percentFormats;
	MonoString *digitPattern;
	MonoString *zeroPattern;
	gint32 currencyDecimalDigits;
	MonoString *currencyDecimalSeparator;
	MonoString *currencyGroupSeparator;
	MonoArray *currencyGroupSizes;
	gint32 currencyNegativePattern;
	gint32 currencyPositivePattern;
	MonoString *currencySymbol;
	MonoString *naNSymbol;
	MonoString *negativeInfinitySymbol;
	MonoString *negativeSign;
	guint32 numberDecimalDigits;
	MonoString *numberDecimalSeparator;
	MonoString *numberGroupSeparator;
	MonoArray *numberGroupSizes;
	gint32 numberNegativePattern;
	gint32 percentDecimalDigits;
	MonoString *percentDecimalSeparator;
	MonoString *percentGroupSeparator;
	MonoArray *percentGroupSizes;
	gint32 percentNegativePattern;
	gint32 percentPositivePattern;
	MonoString *percentSymbol;
	MonoString *perMilleSymbol;
	MonoString *positiveInfinitySymbol;
	MonoString *positiveSign;
} MonoNumberFormatInfo;

typedef struct {
	MonoObject obj;
	gint32 lcid;
	gpointer ICU_collator;
} MonoCompareInfo;

typedef struct {
	MonoObject obj;
	MonoBoolean is_read_only;
	gint32 lcid;
	MonoBoolean use_user_override;
	MonoNumberFormatInfo *number_format;
	MonoDateTimeFormatInfo *datetime_format;
	MonoObject *textinfo;
	MonoString *name;
	MonoString *displayname;
	MonoString *englishname;
	MonoString *nativename;
	MonoString *iso3lang;
	MonoString *iso2lang;
	MonoString *icu_name;
	MonoString *win3lang;
	MonoCompareInfo *compareinfo;
} MonoCultureInfo;

typedef struct {
	MonoObject obj;
	MonoString *str;
	gint32 options;
	MonoArray *key;
	gint32 lcid;
} MonoSortKey;

typedef MonoObject* (*MonoInvokeFunc)        (MonoMethod *method, void *obj, void **params, MonoObject **exc);
typedef gpointer    (*MonoCompileFunc)       (MonoMethod *method);
typedef void	    (*MonoMainThreadFunc)    (gpointer user_data);

#define mono_object_class(obj) (((MonoObject*)(obj))->vtable->klass)
#define mono_object_domain(obj) (((MonoObject*)(obj))->vtable->domain)

#define mono_array_length(array) ((array)->max_length)
#define mono_array_addr(array,type,index) ( ((char*)(array)->vector) + sizeof (type) * (index) )
#define mono_array_addr_with_size(array,size,index) ( ((char*)(array)->vector) + (size) * (index) )
#define mono_array_get(array,type,index) ( *(type*)mono_array_addr ((array), type, (index)) ) 
#define mono_array_set(array,type,index,value)	\
	do {	\
		type *__p = (type *) mono_array_addr ((array), type, (index));	\
		*__p = (value);	\
	} while (0)

#define mono_string_chars(s) ((gunichar2*)(s)->chars)
#define mono_string_length(s) ((s)->length)
#define mono_stringbuilder_capacity(sb) ((sb)->str->length)

MonoObject *
mono_object_new             (MonoDomain *domain, MonoClass *klass);

MonoObject *
mono_object_new_specific    (MonoVTable *vtable);

/* can be used for classes without finalizer in non-profiling mode */
MonoObject *
mono_object_new_fast        (MonoVTable *vtable);

MonoObject *
mono_object_new_alloc_specific (MonoVTable *vtable);

MonoObject *
mono_object_new_from_token  (MonoDomain *domain, MonoImage *image, guint32 token);

MonoArray*
mono_array_new              (MonoDomain *domain, MonoClass *eclass, guint32 n);

MonoArray*
mono_array_new_full         (MonoDomain *domain, MonoClass *array_class,
			     guint32 *lengths, guint32 *lower_bounds);

MonoArray *
mono_array_new_specific     (MonoVTable *vtable, guint32 n);

MonoArray*
mono_array_clone            (MonoArray *array);

MonoString*
mono_string_new_utf16       (MonoDomain *domain, const guint16 *text, gint32 len);

MonoString*
mono_string_new_size	    (MonoDomain *domain, gint32 len);

MonoString*
mono_ldstr                  (MonoDomain *domain, MonoImage *image, guint32 str_index);

MonoString*
mono_string_is_interned     (MonoString *str);

MonoString*
mono_string_intern          (MonoString *str);

MonoString*
mono_string_new             (MonoDomain *domain, const char *text);

MonoString*
mono_string_new_wrapper     (const char *text);

MonoString*
mono_string_new_len         (MonoDomain *domain, const char *text, guint length);

char *
mono_string_to_utf8         (MonoString *string_obj);

gunichar2 *
mono_string_to_utf16        (MonoString *string_obj);

MonoString *
mono_string_from_utf16      (gunichar2 *data);

void       
mono_object_free            (MonoObject *o);

MonoObject *
mono_value_box              (MonoDomain *domain, MonoClass *klass, gpointer val);

gpointer
mono_object_unbox           (MonoObject *obj);

MonoObject *
mono_object_clone           (MonoObject *obj);

MonoObject *
mono_object_isinst          (MonoObject *obj, MonoClass *klass);

MonoObject *
mono_object_isinst_mbyref   (MonoObject *obj, MonoClass *klass);

MonoObject *
mono_object_castclass_mbyref (MonoObject *obj, MonoClass *klass);

typedef void (*MonoExceptionFunc) (MonoException *ex);

void
mono_install_handler        (MonoExceptionFunc func);

void
mono_raise_exception        (MonoException *ex);

void
mono_runtime_object_init    (MonoObject *this_obj);

void
mono_runtime_class_init     (MonoVTable *vtable);

void        
mono_install_runtime_invoke (MonoInvokeFunc func);

void        
mono_install_compile_method (MonoCompileFunc func);

MonoMethod*
mono_object_get_virtual_method (MonoObject *obj, MonoMethod *method);

MonoObject*
mono_runtime_invoke         (MonoMethod *method, void *obj, void **params,
			     MonoObject **exc);

MonoMethod *
mono_get_delegate_invoke    (MonoClass *klass);

MonoObject*
mono_runtime_delegate_invoke (MonoObject *delegate, void **params, 
			      MonoObject **exc);

MonoObject*
mono_runtime_invoke_array   (MonoMethod *method, void *obj, MonoArray *params,
			     MonoObject **exc);

MonoArray*
mono_runtime_get_main_args  (void);

void
mono_runtime_exec_managed_code (MonoDomain *domain,
				MonoMainThreadFunc main_func,
				gpointer main_args);

int
mono_runtime_run_main       (MonoMethod *method, int argc, char* argv[], 
			     MonoObject **exc);

int
mono_runtime_exec_main      (MonoMethod *method, MonoArray *args,
			     MonoObject **exc);

MonoAsyncResult *
mono_async_result_new       (MonoDomain *domain, HANDLE handle, 
			     MonoObject *state, gpointer data);

MonoWaitHandle *
mono_wait_handle_new        (MonoDomain *domain, HANDLE handle);

void
mono_message_init           (MonoDomain *domain, MonoMethodMessage *this_obj, 
			     MonoReflectionMethod *method, MonoArray *out_args);

MonoObject *
mono_remoting_invoke        (MonoObject *real_proxy, MonoMethodMessage *msg, 
			     MonoObject **exc, MonoArray **out_args);

MonoObject *
mono_message_invoke         (MonoObject *target, MonoMethodMessage *msg, 
			     MonoObject **exc, MonoArray **out_args);

MonoMethodMessage *
mono_method_call_message_new (MonoMethod *method, gpointer *params, MonoMethod *invoke, 
			      MonoDelegate **cb, MonoObject **state);

gpointer
mono_load_remote_field (MonoObject *this_obj, MonoClass *klass, MonoClassField *field, gpointer *res);

MonoObject *
mono_load_remote_field_new (MonoObject *this_obj, MonoClass *klass, MonoClassField *field);

void
mono_store_remote_field (MonoObject *this_obj, MonoClass *klass, MonoClassField *field, gpointer val);

void
mono_store_remote_field_new (MonoObject *this_obj, MonoClass *klass, MonoClassField *field, MonoObject *arg);

void
mono_method_return_message_restore (MonoMethod *method, gpointer *params, MonoArray *out_args);

void
mono_unhandled_exception    (MonoObject *exc);

void
mono_print_unhandled_exception (MonoObject *exc);

void
mono_delegate_ctor          (MonoObject *this_obj, MonoObject *target, gpointer addr);

gpointer 
mono_compile_method        (MonoMethod *method);

/* accessors for fields and properties */
void
mono_field_set_value (MonoObject *obj, MonoClassField *field, void *value);

void
mono_field_static_set_value (MonoVTable *vt, MonoClassField *field, void *value);

void
mono_field_get_value (MonoObject *obj, MonoClassField *field, void *value);

void
mono_field_static_get_value (MonoVTable *vt, MonoClassField *field, void *value);

MonoObject *
mono_field_get_value_object (MonoDomain *domain, MonoClassField *field, MonoObject *obj);

void
mono_property_set_value (MonoProperty *prop, void *obj, void **params, MonoObject **exc);

MonoObject*
mono_property_get_value (MonoProperty *prop, void *obj, void **params, MonoObject **exc);

#endif

