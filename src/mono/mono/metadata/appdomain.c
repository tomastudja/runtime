/*
 * appdomain.c: AppDomain functions
 *
 * Authors:
 *	Dietmar Maurer (dietmar@ximian.com)
 *	Patrik Torstensson
 *	Gonzalo Paniagua Javier (gonzalo@ximian.com)
 *
 * (c) 2001-2003 Ximian, Inc. (http://www.ximian.com)
 */

#include <config.h>
#include <glib.h>
#include <string.h>

#include <mono/os/gc_wrapper.h>

#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/socket-io.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/gc-internal.h>
#include <mono/metadata/marshal.h>

HANDLE mono_delegate_semaphore = NULL;
CRITICAL_SECTION mono_delegate_section;

static gunichar2 process_guid [36];
static gboolean process_guid_set = FALSE;

static MonoAssembly *
mono_domain_assembly_preload (MonoAssemblyName *aname,
			      gchar **assemblies_path,
			      gpointer user_data);

static void
mono_domain_fire_assembly_load (MonoAssembly *assembly, gpointer user_data);

static MonoMethod *
look_for_method_by_name (MonoClass *klass, const gchar *name);

/*
 * mono_runtime_init:
 * @domain: domain returned by mono_init ()
 *
 * Initialize the core AppDomain: this function will run also some
 * IL initialization code, so it needs the execution engine to be fully 
 * operational.
 *
 * AppDomain.SetupInformation is set up in mono_runtime_exec_main, where
 * we know the entry_assembly.
 *
 */
void
mono_runtime_init (MonoDomain *domain, MonoThreadStartCB start_cb,
		   MonoThreadAttachCB attach_cb)
{
	MonoAppDomainSetup *setup;
	MonoAppDomain *ad;
	MonoClass *class;

	mono_marshal_init ();
	
	mono_install_assembly_preload_hook (mono_domain_assembly_preload, NULL);
	mono_install_assembly_load_hook (mono_domain_fire_assembly_load, NULL);
	mono_install_lookup_dynamic_token (mono_reflection_lookup_dynamic_token);

	class = mono_class_from_name (mono_defaults.corlib, "System", "AppDomainSetup");
	setup = (MonoAppDomainSetup *) mono_object_new (domain, class);

	class = mono_class_from_name (mono_defaults.corlib, "System", "AppDomain");
	ad = (MonoAppDomain *) mono_object_new (domain, class);
	ad->data = domain;
	domain->domain = ad;
	domain->setup = setup;

	mono_delegate_semaphore = CreateSemaphore (NULL, 0, 0x7fffffff, NULL);
	g_assert (mono_delegate_semaphore != INVALID_HANDLE_VALUE);
	InitializeCriticalSection (&mono_delegate_section);

	mono_context_init (domain);
	mono_context_set (domain->default_context);

	mono_thread_init (start_cb, attach_cb);
	
	/* GC init has to happen after thread init */
	mono_gc_init ();

	mono_network_init ();

	return;
}

void
mono_context_init (MonoDomain *domain)
{
	MonoClass *class;
	MonoAppContext *context;

	class = mono_class_from_name (mono_defaults.corlib, "System.Runtime.Remoting.Contexts", "Context");
	context = (MonoAppContext *) mono_object_new (domain, class);
	context->domain_id = domain->domain_id;
	context->context_id = 0;
	domain->default_context = context;
}

/* This must not be called while there are still running threads executing
 * managed code.
 */
void
mono_runtime_cleanup (MonoDomain *domain)
{
	/* This ends up calling any pending pending (for at most 2 seconds) */
	mono_gc_cleanup ();
	
	mono_network_cleanup ();
}

static MonoDomainFunc quit_function = NULL;

void
mono_runtime_install_cleanup (MonoDomainFunc func)
{
	quit_function = func;
}

void
mono_runtime_quit ()
{
	if (quit_function != NULL)
		quit_function (mono_root_domain, NULL);
}

gboolean
mono_domain_has_type_resolve (MonoDomain *domain)
{
	static MonoClassField *field = NULL;
	MonoObject *o;

	if (field == NULL) {
		MonoClass *klass = mono_defaults.appdomain_class;
		int i;

		for (i = 0; i < klass->field.count; ++i)
			if (strcmp (klass->fields [i].name, "TypeResolve") == 0)
				field = &klass->fields [i];
		g_assert (field);
	}

	mono_field_get_value ((MonoObject*)(domain->domain), field, &o);
	return o != NULL;
}

MonoReflectionAssembly *
mono_domain_try_type_resolve (MonoDomain *domain, char *name, MonoObject *tb)
{
	MonoClass *klass;
	void *params [1];
	static MonoMethod *method = NULL;

	g_assert (domain != NULL && ((name != NULL) || (tb != NULL)));

	if (method == NULL) {
		klass = domain->domain->mbr.obj.vtable->klass;
		g_assert (klass);

		method = look_for_method_by_name (klass, "DoTypeResolve");
		if (method == NULL) {
			g_warning ("Method AppDomain.DoTypeResolve not found.\n");
			return NULL;
		}
	}

	if (name)
		*params = (MonoObject*)mono_string_new (mono_domain_get (), name);
	else
		*params = tb;
	return (MonoReflectionAssembly *) mono_runtime_invoke (method, domain->domain, params, NULL);
}

void
ves_icall_System_AppDomainSetup_InitAppDomainSetup (MonoAppDomainSetup *setup)
{
	MonoDomain* domain = mono_domain_get ();
	MonoAssembly *assembly;
	gchar *str;
	gchar *config_suffix;
	
	MONO_ARCH_SAVE_REGS;

	if (!domain->entry_assembly)
		assembly = mono_root_domain->entry_assembly;
	else 
		assembly = domain->entry_assembly;

	g_assert (assembly);

	setup->application_base = mono_string_new (domain, assembly->basedir);

	config_suffix = g_strconcat (assembly->aname.name, ".exe.config", NULL);
	str = g_build_filename (assembly->basedir, config_suffix, NULL);
	g_free (config_suffix);
	setup->configuration_file = mono_string_new (domain, str);
	g_free (str);
}

MonoObject *
ves_icall_System_AppDomain_GetData (MonoAppDomain *ad, MonoString *name)
{
	MonoDomain *add = ad->data;
	MonoObject *o;
	char *str;

	MONO_ARCH_SAVE_REGS;

	g_assert (ad != NULL);
	g_assert (name != NULL);

	str = mono_string_to_utf8 (name);

	mono_domain_lock (add);

	if (!strcmp (str, "APPBASE"))
		o = (MonoObject *)add->setup->application_base;
	else if (!strcmp (str, "APP_CONFIG_FILE"))
		o = (MonoObject *)add->setup->configuration_file;
	else if (!strcmp (str, "DYNAMIC_BASE"))
		o = (MonoObject *)add->setup->dynamic_base;
	else if (!strcmp (str, "APP_NAME"))
		o = (MonoObject *)add->setup->application_name;
	else if (!strcmp (str, "CACHE_BASE"))
		o = (MonoObject *)add->setup->cache_path;
	else if (!strcmp (str, "PRIVATE_BINPATH"))
		o = (MonoObject *)add->setup->private_bin_path;
	else if (!strcmp (str, "BINPATH_PROBE_ONLY"))
		o = (MonoObject *)add->setup->private_bin_path_probe;
	else if (!strcmp (str, "SHADOW_COPY_DIRS"))
		o = (MonoObject *)add->setup->shadow_copy_directories;
	else if (!strcmp (str, "FORCE_CACHE_INSTALL"))
		o = (MonoObject *)add->setup->shadow_copy_files;
	else 
		o = mono_g_hash_table_lookup (add->env, name);

	mono_domain_unlock (add);
	g_free (str);

	if (!o)
		return NULL;

	return o;
}

void
ves_icall_System_AppDomain_SetData (MonoAppDomain *ad, MonoString *name, MonoObject *data)
{
	MonoDomain *add = ad->data;

	MONO_ARCH_SAVE_REGS;

	g_assert (ad != NULL);
	g_assert (name != NULL);

	mono_domain_lock (add);

	mono_g_hash_table_insert (add->env, name, data);

	mono_domain_unlock (add);
}

MonoAppDomainSetup *
ves_icall_System_AppDomain_getSetup (MonoAppDomain *ad)
{
	MONO_ARCH_SAVE_REGS;

	g_assert (ad != NULL);
	g_assert (ad->data != NULL);

	return ad->data->setup;
}

MonoString *
ves_icall_System_AppDomain_getFriendlyName (MonoAppDomain *ad)
{
	MONO_ARCH_SAVE_REGS;

	g_assert (ad != NULL);
	g_assert (ad->data != NULL);

	return mono_string_new (ad->data, ad->data->friendly_name);
}

MonoAppDomain *
ves_icall_System_AppDomain_getCurDomain ()
{
	MonoDomain *add = mono_domain_get ();

	MONO_ARCH_SAVE_REGS;

	return add->domain;
}

MonoAppDomain *
ves_icall_System_AppDomain_createDomain (MonoString *friendly_name, MonoAppDomainSetup *setup)
{
	/*MonoDomain *domain = mono_domain_get (); */
	MonoClass *adclass;
	MonoAppDomain *ad;
	MonoDomain *data;
	
	MONO_ARCH_SAVE_REGS;

	adclass = mono_class_from_name (mono_defaults.corlib, "System", "AppDomain");

	/* FIXME: pin all those objects */
	data = mono_domain_create();

	ad = (MonoAppDomain *) mono_object_new (data, adclass);
	ad->data = data;
	data->domain = ad;
	data->setup = setup;
	data->friendly_name = mono_string_to_utf8 (friendly_name);

	mono_context_init (data);

	/* FIXME: what to do next ? */

	return ad;
}

typedef struct {
	MonoArray *res;
	MonoDomain *domain;
	int idx;
} add_assembly_helper_t;

static void
add_assembly (gpointer key, gpointer value, gpointer user_data)
{
	add_assembly_helper_t *ah = (add_assembly_helper_t *) user_data;

	mono_array_set (ah->res, gpointer, ah->idx++, mono_assembly_get_object (ah->domain, value));
}

MonoArray *
ves_icall_System_AppDomain_GetAssemblies (MonoAppDomain *ad)
{
	MonoDomain *domain = ad->data; 
	static MonoClass *System_Reflection_Assembly;
	MonoArray *res;
	add_assembly_helper_t ah;
	
	MONO_ARCH_SAVE_REGS;

	if (!System_Reflection_Assembly)
		System_Reflection_Assembly = mono_class_from_name (
			mono_defaults.corlib, "System.Reflection", "Assembly");

	res = mono_array_new (domain, System_Reflection_Assembly, g_hash_table_size (domain->assemblies));

	ah.domain = domain;
	ah.res = res;
	ah.idx = 0;
	mono_domain_lock (domain);
	g_hash_table_foreach (domain->assemblies, add_assembly, &ah);
	mono_domain_unlock (domain);

	return res;
}

/*
 * Used to find methods in AppDomain class.
 * It only works if there are no multiple signatures for any given method name
 */
static MonoMethod *
look_for_method_by_name (MonoClass *klass, const gchar *name)
{
	gint i;
	MonoMethod *method;

	for (i = 0; i < klass->method.count; i++) {
		method = klass->methods [i];
		if (!strcmp (method->name, name))
			return method;
	}

	return NULL;
}

static MonoReflectionAssembly *
try_assembly_resolve (MonoDomain *domain, MonoString *fname)
{
	MonoClass *klass;
	MonoMethod *method;
	void *params [1];

	g_assert (domain != NULL && fname != NULL);

	klass = domain->domain->mbr.obj.vtable->klass;
	g_assert (klass);
	
	method = look_for_method_by_name (klass, "DoAssemblyResolve");
	if (method == NULL) {
		g_warning ("Method AppDomain.DoAssemblyResolve not found.\n");
		return NULL;
	}

	*params = fname;
	return (MonoReflectionAssembly *) mono_runtime_invoke (method, domain->domain, params, NULL);
}

static void
add_assemblies_to_domain (MonoDomain *domain, MonoAssembly *ass)
{
	gint i;

	if (g_hash_table_lookup (domain->assemblies, ass->aname.name))
		return; /* This is ok while no lazy loading of assemblies */

	mono_domain_lock (domain);
	g_hash_table_insert (domain->assemblies, (gpointer) ass->aname.name, ass);
	mono_domain_unlock (domain);

	if (ass->image->references)
		for (i = 0; ass->image->references [i] != NULL; i++)
			add_assemblies_to_domain (domain, ass->image->references [i]);
}

static void
mono_domain_fire_assembly_load (MonoAssembly *assembly, gpointer user_data)
{
	MonoDomain *domain = mono_domain_get ();
	MonoReflectionAssembly *ref_assembly;
	MonoClass *klass;
	MonoMethod *method;
	void *params [1];

	klass = domain->domain->mbr.obj.vtable->klass;

	
	method = look_for_method_by_name (klass, "DoAssemblyLoad");
	if (method == NULL) {
		g_warning ("Method AppDomain.DoAssemblyLoad not found.\n");
		return;
	}

	add_assemblies_to_domain (domain, assembly);

	ref_assembly = mono_assembly_get_object (domain, assembly);
	g_assert (ref_assembly);

	*params = ref_assembly;
	mono_runtime_invoke (method, domain->domain, params, NULL);
}

static void
set_domain_search_path (MonoDomain *domain)
{
	MonoAppDomainSetup *setup;
	gchar **tmp;
	gchar *utf8;
	gint i;
	gint npaths = 0;
	gchar **pvt_split = NULL;
	GError *error = NULL;

	if ((domain->search_path != NULL) && !domain->setup->path_changed)
		return;

	setup = domain->setup;
	if (setup->application_base)
		npaths++;

	if (setup->private_bin_path) {
		utf8 = mono_string_to_utf8 (setup->private_bin_path);
		pvt_split = g_strsplit (utf8, G_SEARCHPATH_SEPARATOR_S, 1000);
		g_free (utf8);
		for (tmp = pvt_split; *tmp; tmp++, npaths++);
	}

	if (!npaths) {
		if (pvt_split)
			g_strfreev (pvt_split);
		/*
		 * Don't do this because the first time is called, the domain
		 * setup is not finished.
		 *
		 * domain->search_path = g_malloc (sizeof (char *));
		 * domain->search_path [0] = NULL;
		*/
		return;
	}

	if (domain->search_path)
		g_strfreev (domain->search_path);

	domain->search_path = tmp = g_malloc ((npaths + 1) * sizeof (gchar *));
	tmp [npaths] = NULL;
	if (setup->application_base) {
		*tmp = mono_string_to_utf8 (setup->application_base);

		/* FIXME: is this needed? */
		if (strncmp (*tmp, "file://", 7) == 0) {
			gchar *file = *tmp;
			gchar *uri = *tmp;

			if (uri [7] != '/')
				uri = g_strdup_printf ("file:///%s", uri + 7);

			*tmp = g_filename_from_uri (uri, NULL, &error);
			if (uri != file)
				g_free (uri);

			if (error != NULL) {
				g_warning ("%s\n", error->message);
				g_error_free (error);
				*tmp = file;
			} else {
				g_free (file);
			}
		}
		
	} else {
		*tmp = g_strdup ("");
	}

	for (i = 1; pvt_split && i < npaths; i++) {
		if (*tmp [0] == '\0' || g_path_is_absolute (pvt_split [i - 1])) {
			tmp [i] = g_strdup (pvt_split [i - 1]);
			continue;
		}

		tmp [i] = g_build_filename (tmp [0], pvt_split [i - 1], NULL);
	}
	
	if (setup->private_bin_path_probe != NULL && setup->application_base) {
		g_free (tmp [0]);
		tmp [0] = g_strdup ("");
	}
		
	domain->setup->path_changed = FALSE;

	g_strfreev (pvt_split);
}

static MonoAssembly *
real_load (gchar **search_path, gchar *filename)
{
	MonoAssembly *result;
	gchar **path;
	gchar *fullpath;

	for (path = search_path; *path; path++) {
		if (**path == '\0')
			continue; /* Ignore empty ApplicationBase */
		fullpath = g_build_filename (*path, filename, NULL);
		result = mono_assembly_open (fullpath, NULL);
		g_free (fullpath);
		if (result)
			return result;
	}

	return NULL;
}

/*
 * Try loading the assembly from ApplicationBase and PrivateBinPath 
 * and then from assemblies_path if any.
 */
static MonoAssembly *
mono_domain_assembly_preload (MonoAssemblyName *aname,
			      gchar **assemblies_path,
			      gpointer user_data)
{
	MonoDomain *domain = mono_domain_get ();
	MonoAssembly *result;
	gchar *dll, *exe;

	set_domain_search_path (domain);

	dll = g_strconcat (aname->name, ".dll", NULL);
	exe = g_strdup (dll);
	strcpy (exe + strlen (exe) - 4, ".exe");

	if (domain->search_path && domain->search_path [0] != NULL) {
		/* TODO: should also search in name/name.dll and name/name.exe from appbase */
		result = real_load (domain->search_path, dll);
		if (result) {
			g_free (dll);
			g_free (exe);
			return result;
		}

		result = real_load (domain->search_path, exe);
		if (result) {
			g_free (dll);
			g_free (exe);
			return result;
		}
	}

	if (assemblies_path && assemblies_path [0] != NULL) {
		result = real_load (assemblies_path, dll);
		if (result) {
			g_free (dll);
			g_free (exe);
			return result;
		}

		result = real_load (assemblies_path, exe);
		if (result) {
			g_free (dll);
			g_free (exe);
			return result;
		}
	}
	
	g_free (dll);
	g_free (exe);
	return NULL;
}

MonoReflectionAssembly *
ves_icall_System_Reflection_Assembly_LoadFrom (MonoString *fname)
{
	MonoDomain *domain = mono_domain_get ();
	char *name, *filename;
	MonoImageOpenStatus status = MONO_IMAGE_OK;
	MonoAssembly *ass;

	MONO_ARCH_SAVE_REGS;

	if (fname == NULL) {
		MonoException *exc = mono_get_exception_argument_null ("assemblyFile");
		mono_raise_exception (exc);
	}
		
	name = filename = mono_string_to_utf8 (fname);

	ass = mono_assembly_open (filename, &status);
	
	g_free (name);

	if (!ass){
		MonoException *exc = mono_get_exception_file_not_found (fname);
		mono_raise_exception (exc);
	}

	return mono_assembly_get_object (domain, ass);
}

static void
free_assembly_name (MonoAssemblyName *aname)
{
	if (aname == NULL)
		return;

	g_free ((void *) aname->name);
	g_free ((void *) aname->culture);
	g_free ((void *) aname->hash_value);
}

static gboolean
get_info_from_assembly_name (MonoReflectionAssemblyName *assRef, MonoAssemblyName *aname)
{
	gchar *name;
	gchar *value;
	gchar **parts;
	gchar **tmp;
	gint major, minor, build, revision;

	memset (aname, 0, sizeof (MonoAssemblyName));

	name = mono_string_to_utf8 (assRef->name);
	parts = tmp = g_strsplit (name, ",", 4);
	g_free (name);
	if (!tmp || !*tmp) {
		g_strfreev (tmp);
		return FALSE;
	}

	value = g_strstrip (*tmp);
	/* g_print ("Assembly name: %s\n", value); */
	aname->name = g_strdup (value);
	tmp++;
	if (!*tmp) {
		g_strfreev (parts);
		return TRUE;
	}

	value = g_strstrip (*tmp);
	if (strncmp (value, "Version=", 8)) {
		g_strfreev (parts);
		return FALSE;
	}
	
	if (sscanf (value + 8, "%u.%u.%u.%u", &major, &minor, &build, &revision) != 4) {
		g_strfreev (parts);
		return FALSE;
	}

	/* g_print ("Version: %u.%u.%u.%u\n", major, minor, build, revision); */
	aname->major = major;
	aname->minor = minor;
	aname->build = build;
	aname->revision = revision;
	tmp++;

	if (!*tmp) {
		g_strfreev (parts);
		return FALSE;
	}

	value = g_strstrip (*tmp);
	if (strncmp (value, "Culture=", 8)) {
		g_strfreev (parts);
		return FALSE;
	}

	/* g_print ("Culture: %s\n", aname->culture); */
	aname->culture = g_strstrip (g_strdup (value + 8));
	tmp++;

	if (!*tmp) {
		g_strfreev (parts);
		return FALSE;
	}

	value = g_strstrip (*tmp);
	if (strncmp (value, "PublicKeyToken=", 15)) {
		g_strfreev (parts);
		return FALSE;
	}

	value += 15;
	if (*value && strcmp (value, "null")) {
		gint i, len;
		gchar h, l;
		gchar *result;
		
		value = g_strstrip (g_strdup (value));
		len = strlen (value);
		if (len % 2) {
			g_free (value);
			g_strfreev (parts);
			return FALSE;
		}
		
		aname->hash_len = len / 2;
		aname->hash_value = g_malloc0 (aname->hash_len);
		result = (gchar *) aname->hash_value;
		
		for (i = 0; i < len; i++) {
			if (i % 2) {
				l = g_ascii_xdigit_value (value [i]);
				if (l == -1) {
					g_free (value);
					g_strfreev (parts);
					return FALSE;
				}
				result [i / 2] = (h * 16) + l;
			} else {
				h = g_ascii_xdigit_value (value [i]);
				if (h == -1) {
					g_free (value);
					g_strfreev (parts);
					return FALSE;
				}
			}
		}
		g_free (value);

		/*
		g_print ("PublicKeyToken: ");
		for (i = 0; i < aname->hash_len; i++) {
			g_print ("%x", 0x00FF & aname->hash_value [i]); 
		}
		g_print ("\n");
		*/
	}

	g_strfreev (parts);
	return TRUE;
}

MonoReflectionAssembly *
ves_icall_System_AppDomain_LoadAssembly (MonoAppDomain *ad,  MonoReflectionAssemblyName *assRef, MonoObject *evidence)
{
	MonoDomain *domain = ad->data; 
	MonoImageOpenStatus status = MONO_IMAGE_OK;
	MonoAssembly *ass;
	MonoAssemblyName aname;
	MonoReflectionAssembly *refass = NULL;

	MONO_ARCH_SAVE_REGS;

	memset (&aname, 0, sizeof (aname));

	/* FIXME : examine evidence? */

	g_assert (assRef != NULL);
	g_assert (assRef->name != NULL);

	if (!get_info_from_assembly_name (assRef, &aname)) {
		MonoException *exc;

		free_assembly_name (&aname);
		/* This is a parse error... */
		exc = mono_get_exception_file_not_found (assRef->name);
		mono_raise_exception (exc);
	}

	ass = mono_assembly_load (&aname, NULL, &status);
	free_assembly_name (&aname);

	if (!ass && (refass = try_assembly_resolve (domain, assRef->name)) == NULL){
		/* FIXME: it doesn't make much sense since we really don't have a filename ... */
		MonoException *exc = mono_get_exception_file_not_found (assRef->name);
		mono_raise_exception (exc);
	}

	if (refass != NULL)
		return refass;

	return mono_assembly_get_object (domain, ass);
}

void
ves_icall_System_AppDomain_InternalUnload (gint32 domain_id)
{
	MonoDomain * domain = mono_domain_get_by_id (domain_id);

	MONO_ARCH_SAVE_REGS;

	if (NULL == domain) {
		MonoException *exc = mono_get_exception_execution_engine ("Failed to unload domain, domain id not found");
		mono_raise_exception (exc);
	}
	

	mono_domain_unload (domain, FALSE);
}

gint32
ves_icall_System_AppDomain_ExecuteAssembly (MonoAppDomain *ad, MonoString *file, 
					    MonoObject *evidence, MonoArray *args)
{
	MonoAssembly *assembly;
	MonoImage *image;
	MonoMethod *method;
	char *filename;
	gint32 res;

	MONO_ARCH_SAVE_REGS;

	filename = mono_string_to_utf8 (file);
	assembly = mono_assembly_open (filename, NULL);
	g_free (filename);

	if (!assembly) {
		mono_raise_exception ((MonoException *)mono_exception_from_name (
		        mono_defaults.corlib, "System.IO", "FileNotFoundException"));
	}

	image = assembly->image;

	method = mono_get_method (image, mono_image_get_entry_point (image), NULL);

	if (!method)
		g_error ("No entry point method found in %s", image->name);

	if (!args)
		args = (MonoArray *) mono_array_new (ad->data, mono_defaults.string_class, 0);

	res = mono_runtime_exec_main (method, (MonoArray *)args, NULL);

	return res;
}

gint32 
ves_icall_System_AppDomain_GetIDFromDomain (MonoAppDomain * ad) 
{
	MONO_ARCH_SAVE_REGS;

	return ad->data->domain_id;
}

MonoAppDomain * 
ves_icall_System_AppDomain_InternalSetDomain (MonoAppDomain *ad)
{
	MonoDomain *old_domain = mono_domain_get();

	MONO_ARCH_SAVE_REGS;

	mono_domain_set(ad->data);

	return old_domain->domain;
}

MonoAppDomain * 
ves_icall_System_AppDomain_InternalSetDomainByID (gint32 domainid)
{
	MonoDomain *current_domain = mono_domain_get ();
	MonoDomain *domain = mono_domain_get_by_id (domainid);

	MONO_ARCH_SAVE_REGS;

	mono_domain_set (domain);
	
	return current_domain->domain;
}

MonoAppContext * 
ves_icall_System_AppDomain_InternalGetContext ()
{
	MONO_ARCH_SAVE_REGS;

	return mono_context_get ();
}

MonoAppContext * 
ves_icall_System_AppDomain_InternalGetDefaultContext ()
{
	MONO_ARCH_SAVE_REGS;

	return mono_domain_get ()->default_context;
}

MonoAppContext * 
ves_icall_System_AppDomain_InternalSetContext (MonoAppContext *mc)
{
	MonoAppContext *old_context = mono_context_get ();

	MONO_ARCH_SAVE_REGS;

	mono_context_set (mc);
	
	return old_context;
}

MonoString *
ves_icall_System_AppDomain_InternalGetProcessGuid (MonoString* newguid)
{
	mono_domain_lock (mono_root_domain);
	if (process_guid_set) {
		mono_domain_unlock (mono_root_domain);
		return mono_string_new_utf16 (mono_domain_get (), process_guid, sizeof(process_guid)/2);
	}
	memcpy (process_guid, mono_string_chars(newguid), sizeof(process_guid));
	process_guid_set = TRUE;
	mono_domain_unlock (mono_root_domain);
	return newguid;
}
