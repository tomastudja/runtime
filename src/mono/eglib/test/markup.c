#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "test.h"

#define do_bad_test(s) do { char *r = markup_test (s); if (r == NULL) return FAILED ("Failed on test " # s); } while (0)
#define do_ok_test(s) do { char *r = markup_test (s); if (r != NULL) return FAILED ("Could not parse valid " # s); } while (0)

static char *
markup_test (const char *s)
{
	GMarkupParser *parser = g_new0 (GMarkupParser, 1);
	GMarkupParseContext *context;
	GError *error = NULL;
	
	context = g_markup_parse_context_new (parser, 0, 0, 0);

	g_markup_parse_context_parse (context, s, strlen (s), &error);
	g_markup_parse_context_free (context);

	if (error != NULL){
		char *msg = g_strdup (error->message);
		g_error_free (error);

		return msg;
	}
	return NULL;
}

RESULT
invalid_documents (void)
{
	/* These should fail */
	do_bad_test ("<1>");
	do_bad_test ("<a<");
	do_bad_test ("</a>");
	do_bad_test ("<a b>");
	do_bad_test ("<a b=>");
	do_bad_test ("<a b=c>");
	
	return OK;
}

RESULT
valid_documents (void)
{
	/* These should fail */
	do_ok_test ("<a>");
	do_ok_test ("<a a=\"b\">");
	
	return OK;
}

/*
 * This is a test for the kind of files that the code in mono/domain.c
 * parses;  This code comes from Mono
 */
typedef struct {
        GSList *supported_runtimes;
        char *required_runtime;
        int configuration_count;
        int startup_count;
} AppConfigInfo;

static char *
get_attribute_value (const gchar **attribute_names,
		     const gchar **attribute_values,
		     const char *att_name)
{
        int n;
        for (n=0; attribute_names[n] != NULL; n++) {
                if (strcmp (attribute_names[n], att_name) == 0)
                        return g_strdup (attribute_values[n]);
        }
        return NULL;
}

static void
start_element (GMarkupParseContext *context,
	       const gchar         *element_name,
	       const gchar        **attribute_names,
	       const gchar        **attribute_values,
	       gpointer             user_data,
	       GError             **error)
{
        AppConfigInfo* app_config = (AppConfigInfo*) user_data;

        if (strcmp (element_name, "configuration") == 0) {
                app_config->configuration_count++;
                return;
        }
        if (strcmp (element_name, "startup") == 0) {
                app_config->startup_count++;
                return;
        }

        if (app_config->configuration_count != 1 || app_config->startup_count != 1)
                return;

        if (strcmp (element_name, "requiredRuntime") == 0) {
                app_config->required_runtime = get_attribute_value (attribute_names, attribute_values, "version");
        } else if (strcmp (element_name, "supportedRuntime") == 0) {
                char *version = get_attribute_value (attribute_names, attribute_values, "version");
                app_config->supported_runtimes = g_slist_append (app_config->supported_runtimes, version);
        }
}

static void
end_element   (GMarkupParseContext *context,
	       const gchar         *element_name,
	       gpointer             user_data,
	       GError             **error)
{
        AppConfigInfo* app_config = (AppConfigInfo*) user_data;
	
        if (strcmp (element_name, "configuration") == 0) {
                app_config->configuration_count--;
        } else if (strcmp (element_name, "startup") == 0) {
                app_config->startup_count--;
        }
}

static const GMarkupParser
mono_parser = {
        start_element,
        end_element,
        NULL,
        NULL,
        NULL
};

AppConfigInfo *
domain_test (char *text)
{
	AppConfigInfo *app_config = g_new0 (AppConfigInfo, 1);
	GMarkupParseContext *context;
	
        context = g_markup_parse_context_new (&mono_parser, 0, app_config, NULL);
        if (g_markup_parse_context_parse (context, text, strlen (text), NULL)) {
                g_markup_parse_context_end_parse (context, NULL);
        }
        g_markup_parse_context_free (context);

	return app_config;
}

void
domain_free (AppConfigInfo *info)
{
	GSList *l;
	if (info->required_runtime)
		g_free (info->required_runtime);
	for (l = info->supported_runtimes; l != NULL; l = l->next){
		g_free (l->data);
	}
	g_slist_free (info->supported_runtimes);
	g_free (info);
}

RESULT
mono_domain (void)
{
	AppConfigInfo *info;

	info = domain_test ("<configuration><!--hello--><startup><!--world--><requiredRuntime version=\"v1\"><!--r--></requiredRuntime></startup></configuration>");
	if (info->required_runtime == NULL)
		return FAILED ("No required runtime section");
	if (strcmp (info->required_runtime, "v1") != 0)
		return FAILED ("Got a runtime version %s, expected v1", info->required_runtime);
	domain_free (info);

	info = domain_test ("<configuration><startup><requiredRuntime version=\"v1\"/><!--comment--></configuration><!--end-->");
	if (info->required_runtime == NULL)
		return FAILED ("No required runtime section on auto-close section");
	if (strcmp (info->required_runtime, "v1") != 0)
		return FAILED ("Got a runtime version %s, expected v1", info->required_runtime);
	domain_free (info);

	info = domain_test ("<!--start--><configuration><startup><supportedRuntime version=\"v1\"/><!--middle--><supportedRuntime version=\"v2\"/></startup></configuration>");
	if ((strcmp ((char*)info->supported_runtimes->data, "v1") == 0)){
		if (info->supported_runtimes->next == NULL)
			return FAILED ("Expected 2 supported runtimes");
		
		if ((strcmp ((char*)info->supported_runtimes->next->data, "v2") != 0))
			return FAILED ("Expected v1, v2, got %s", info->supported_runtimes->next->data);
		if (info->supported_runtimes->next->next != NULL)
			return FAILED ("Expected v1, v2, got more");
	} else
		return FAILED ("Expected `v1', got %s", info->supported_runtimes->data);
	domain_free (info);

	return NULL;
}

static Test markup_tests [] = {
	{"invalid_documents", invalid_documents},
	{"good_documents", valid_documents},
	{"mono_domain", mono_domain},
	{NULL, NULL}
};

DEFINE_TEST_GROUP_INIT(markup_tests_init, markup_tests)

