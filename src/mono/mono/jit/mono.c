/*
 * mono.c: Main driver for the Mono JIT engine
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * (C) 2001, 2002 Ximian, Inc (http://www.ximian.com)
 */
#include "jit.h"
#include "regset.h"
#include "codegen.h"
#include "mono/metadata/debug-helpers.h"
#include "mono/metadata/verify.h"
#include "mono/metadata/profiler.h"
#include "mono/metadata/threadpool.h"
#include "mono/metadata/mono-config.h"
#include <mono/metadata/profiler-private.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-debug-debugger.h>
#include <mono/os/util.h>
#include <locale.h>

static MonoClass *
find_class_in_assembly (MonoAssembly *assembly, const char *namespace, const char *name)
{
	MonoAssembly **ptr;
	MonoClass *class;

	class = mono_class_from_name (assembly->image, namespace, name);
	if (class)
		return class;

	for (ptr = assembly->image->references; ptr && *ptr; ptr++) {
		class = find_class_in_assembly (*ptr, namespace, name);
		if (class)
			return class;
	}

	return NULL;
}

static MonoMethod *
find_method_in_assembly (MonoAssembly *assembly, MonoMethodDesc *mdesc)
{
	MonoAssembly **ptr;
	MonoMethod *method;

	method = mono_method_desc_search_in_image (mdesc, assembly->image);
	if (method)
		return method;

	for (ptr = assembly->image->references; ptr && *ptr; ptr++) {
		method = find_method_in_assembly (*ptr, mdesc);
		if (method)
			return method;
	}

	return NULL;
}

/**
 * mono_jit_compile_class:
 * @assembly: Lookup things in this assembly
 * @compile_class: Name of the image/class/method to compile
 * @compile_times: Compile it that many times
 * @verbose: If true, print debugging information on stdout.
 *
 * JIT compilation of the image/class/method.
 *
 * @compile_class can be one of:
 *
 * - an image name (`@corlib')
 * - a class name (`System.Int32')
 * - a method name (`System.Int32:Parse')
 */
void
mono_jit_compile_class (MonoAssembly *assembly, char *compile_class,
			int compile_times, int verbose)
{
	const char *cmethod = strrchr (compile_class, ':');
	char *cname;
	char *code;
	int i, j;
	MonoClass *class;
	MonoDomain *domain = mono_domain_get ();

	if (compile_class [0] == '@') {
		MonoImage *image = mono_image_loaded (compile_class + 1);
		if (!image)
			g_error ("Cannot find image %s", compile_class + 1);
		if (verbose)
			g_print ("Compiling image %s\n", image->name);
		for (j = 0; j < compile_times; ++j)
			mono_jit_compile_image (image, verbose);
	} else if (cmethod) {
		MonoMethodDesc *mdesc;
		MonoMethod *m;
		mdesc = mono_method_desc_new (compile_class, FALSE);
		if (!mdesc)
			g_error ("Invalid method name '%s'", compile_class);
		m = find_method_in_assembly (assembly, mdesc);
		if (!m)
			g_error ("Cannot find method '%s'", compile_class);
		for (j = 0; j < compile_times; ++j) {
			code = mono_compile_method (m);
			// g_free (code);
			g_hash_table_remove (domain->jit_code_hash, m);
		}
	} else {
		cname = strrchr (compile_class, '.');
		if (cname)
			*cname++ = 0;
		else {
			cname = compile_class;
			compile_class = (char *)"";
		}
		class = find_class_in_assembly (assembly, compile_class, cname);
		if (!class)
			g_error ("Cannot find class %s.%s", compile_class, cname);
		mono_class_init (class);
		for (j = 0; j < compile_times; ++j) {
			for (i = 0; i < class->method.count; ++i) {
				if (class->methods [i]->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL)
					continue;
				if (class->methods [i]->flags & METHOD_ATTRIBUTE_ABSTRACT)
					continue;
				if (verbose)
					g_print ("Compiling: %s.%s:%s\n",
						 compile_class, cname, class->methods [i]->name);
				code = mono_compile_method (class->methods [i]);
				g_hash_table_remove (domain->jit_code_hash, class->methods [i]);
				// g_free (code);
			}
		}
	}
}

static void
usage (char *name)
{
	fprintf (stderr,
		 "mono %s, the Mono ECMA CLI JIT Compiler, (C) 2001, 2002 Ximian, Inc.\n\n"
		 "Usage is: %s [options] executable args...\n\n",  VERSION, name);
	fprintf (stderr,
		 "Runtime Debugging:\n"
		 "    -d                 debug the jit, show disassembler output.\n"
		 "    --dump-asm         dumps the assembly code generated\n"
		 "    --dump-forest      dumps the reconstructed forest\n"
		 "    --print-vtable     print the VTable of all used classes\n"
		 "    --stats            print statistics about the jit operations\n"
		 "    --trace            printf function call trace\n"
		 "    --compile NAME     compile NAME, then exit.\n"
		 "                       NAME is in one of the following formats:\n"
		 "                         namespace.name          compile the given class\n"
		 "                         namespace.name:method   compile the given method\n"
		 "                         @imagename              compile the given image\n"
		 "    --ncompile NUM     compile methods NUM times (default: 1000)\n"
		 "    --noboundcheck     Disables bound checks\n"
		 "\n"
		 "Development:\n"
		 "    --debug            enable debugging support.\n"
		 "    --profile          record and dump profile info\n"
		 "    --breakonex        set a breakpoint for unhandled exception\n"
		 "    --break NAME       insert a breakpoint at the start of method NAME\n"
		 "                       (NAME is in `namespace.name:methodname' format\n"
		 "                       or `Main' to break on the application's main method)\n"
		 "    --precompile name  precompile NAME before executing the main application:\n"
		 "                       NAME is in one of the following formats:\n"
		 "                         namespace.name          compile the given class\n"
		 "                         namespace.name:method   compile the given method\n"
		 "                         @imagename              compile the given image\n"
		 "\n"
		 "Runtime:\n"
		 "    --config filename  Load specified config file instead of the default.\n"
		 "    --fast-iconv       Use fast floating point integer conversion\n"
		 "    --noinline         Disable code inliner\n"
		 "    --nointrinsic      Disable memcopy inliner\n"
		 "    --nols             disable linear scan register allocation\n"
		 "    --share-code       force jit to produce shared code\n"
		 "    --workers n        maximum number of worker threads\n"
		);
	exit (1);
}

typedef struct 
{
	MonoDomain *domain;
	char *file;
	gboolean testjit;
	gboolean enable_debugging;
	char *compile_class;
	int compile_times;
	GList *precompile_classes;
	int verbose;
	int break_on_main;
	int argc;
	char **argv;
} MainThreadArgs;

static void main_thread_handler (gpointer user_data)
{
	MainThreadArgs *main_args=(MainThreadArgs *)user_data;
	MonoAssembly *assembly;

	if (main_args->enable_debugging)
		mono_debug_init (MONO_DEBUG_FORMAT_MONO);

	assembly = mono_domain_assembly_open (main_args->domain,
					      main_args->file);
	if (!assembly){
		fprintf (stderr, "Can not open image %s\n", main_args->file);
		exit (1);
	}

	if (main_args->enable_debugging)
		mono_debug_init_2 (assembly);

	if (main_args->testjit) {
		mono_jit_compile_image (assembly->image, TRUE);
	} else if (main_args->compile_class) {
		mono_jit_compile_class (assembly, main_args->compile_class, main_args->compile_times, TRUE);
	} else {
		GList *tmp;

		for (tmp = main_args->precompile_classes; tmp; tmp = tmp->next)
			mono_jit_compile_class (assembly, tmp->data, 1,
						main_args->verbose);

		if (main_args->break_on_main) {
			MonoImage *image = assembly->image;
			MonoMethodDesc *desc;
			MonoMethod *method;

			method = mono_get_method (image, mono_image_get_entry_point (image), NULL);
			desc = mono_method_desc_from_method (method);
			mono_debugger_insert_breakpoint_full (desc);
		}

		mono_jit_exec (main_args->domain, assembly, main_args->argc,
			       main_args->argv);
	}
}

int 
main (int argc, char *argv [])
{
	MonoDomain *domain;
	int retval = 0, i;
	int compile_times = 1000;
	char *compile_class = NULL;
	gboolean enable_debugging = FALSE;
	char *file, *error, *config_file = NULL;
	gboolean testjit = FALSE;
	int verbose = FALSE;
	GList *precompile_classes = NULL;
	int break_on_main = FALSE;
	MainThreadArgs main_args;
	
	setlocale(LC_ALL, "");
	g_log_set_always_fatal (G_LOG_LEVEL_ERROR);
	g_log_set_fatal_mask (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR);
	
	if (argc < 2)
		usage (argv [0]);

	for (i = 1; i < argc && argv [i][0] == '-'; i++){
		if (strcmp (argv [i], "--help") == 0) {
			usage (argv [0]);
		} else if (strcmp (argv [i], "-d") == 0) {
			testjit = TRUE;
			mono_jit_dump_asm = TRUE;
			mono_jit_dump_forest = TRUE;
		} else if (strcmp (argv [i], "--dump-asm") == 0)
			mono_jit_dump_asm = TRUE;
		else if (strcmp (argv [i], "--dump-forest") == 0)
			mono_jit_dump_forest = TRUE;
		else if (strcmp (argv [i], "--trace") == 0)
			mono_jit_trace_calls = TRUE;
		else if (strcmp (argv [i], "--share-code") == 0)
			mono_jit_share_code = TRUE;
		else if (strcmp (argv [i], "--noinline") == 0)
			mono_jit_inline_code = FALSE;
		else if (strcmp (argv [i], "--nointrinsic") == 0)
			mono_inline_memcpy = FALSE;
		else if (strcmp (argv [i], "--noboundcheck") == 0)
			mono_jit_boundcheck = FALSE;
		else if (strcmp (argv [i], "--nols") == 0)
			mono_use_linear_scan = FALSE;
		else if (strcmp (argv [i], "--breakonex") == 0)
			mono_break_on_exc = TRUE;
		else if (strcmp (argv [i], "--print-vtable") == 0)
			mono_print_vtable = TRUE;
		else if (strcmp (argv [i], "--break") == 0) {
			if (!strcmp (argv [i+1], "Main")) {
				break_on_main = TRUE;
				i++;
			} else {
				if (!mono_debugger_insert_breakpoint (argv [++i], FALSE))
					g_error ("Invalid method name '%s'", argv [i]);
			}
		} else if (strcmp (argv [i], "--count") == 0) {
			compile_times = atoi (argv [++i]);
		} else if (strcmp (argv [i], "--config") == 0) {
			config_file = argv [++i];
		} else if (strcmp (argv [i], "--workers") == 0) {
			mono_worker_threads = atoi (argv [++i]);
			if (mono_worker_threads < 1)
				mono_worker_threads = 1;
		} else if (strcmp (argv [i], "--profile") == 0) {
			mono_jit_profile = TRUE;
			mono_profiler_install_simple ();
		} else if (strcmp (argv [i], "--compile") == 0) {
			compile_class = argv [++i];
		} else if (strcmp (argv [i], "--ncompile") == 0) {
			compile_times = atoi (argv [++i]);
		} else if (strcmp (argv [i], "--stats") == 0) {
			memset (&mono_jit_stats, 0, sizeof (MonoJitStats));
			mono_jit_stats.enabled = TRUE;
		} else if (strcmp (argv [i], "--debug") == 0) {
			enable_debugging = TRUE;
		} else if (strcmp (argv [i], "--precompile") == 0) {
			precompile_classes = g_list_append (precompile_classes, argv [++i]);
		} else if (strcmp (argv [i], "--verbose") == 0) {
			verbose = TRUE;;
		} else if (strcmp (argv [i], "--fast-iconv") == 0) {
			mono_use_fast_iconv = TRUE;
		} else
			usage (argv [0]);
	}
	
	file = argv [i];

	if (!file)
		usage (argv [0]);

	g_set_prgname (file);
	mono_config_parse (config_file);
	mono_set_rootdir ();

	domain = mono_jit_init (file);

	error = mono_verify_corlib ();
	if (error) {
		fprintf (stderr, "Corlib not in sync with this runtime: %s\n", error);
		exit (1);
	}
	
	main_args.domain=domain;
	main_args.file=file;
	main_args.testjit=testjit;
	main_args.enable_debugging=enable_debugging;
	main_args.compile_class=compile_class;
	main_args.compile_times=compile_times;
	main_args.precompile_classes=precompile_classes;
	main_args.verbose=verbose;
	main_args.break_on_main=break_on_main;
	main_args.argc=argc-i;
	main_args.argv=argv+i;
	
	mono_runtime_exec_managed_code (domain, main_thread_handler,
					&main_args);

	mono_profiler_shutdown ();
	mono_jit_cleanup (domain);

	/* Look up return value from System.Environment.ExitCode */
	retval=mono_environment_exitcode_get ();
	
	return retval;
}
