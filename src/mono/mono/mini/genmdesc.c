/*
 * genmdesc: Generates the machine description
 *
 * Authors:
 *   Paolo Molaro (lupus@ximian.com)
 *
 * (C) 2003 Ximian, Inc.
 */
#include "mini.h"
#include <ctype.h>
#include <string.h>
#include <mono/metadata/opcodes.h>

typedef struct {
	int num;
	const char *name;
	char *desc;
	char *comment;
	char spec [MONO_INST_MAX];
} OpDesc;

static GHashTable *table;

#define eat_whitespace(s) while (*(s) && isspace (*(s))) s++;

static int
load_file (const char *name) {
	FILE *f;
	char buf [256];
	char *str, *p;
	int line;
	OpDesc *desc;
	GString *comment;

	if (!(f = fopen (name, "r")))
		g_error ("Cannot open file '%s'", name);

	comment = g_string_new ("");
	/*
	 * The format of the lines are:
	 * # comment
	 * opcode: [dest:format] [src1:format] [src2:format] [flags:format] [clob:format] 
	 * 	[cost:num] [res:format] [delay:num] [len:num]
	 * format is a single letter that depends on the field
	 * NOTE: no space between the field name and the ':'
	 *
	 * len: maximum instruction length
	 */
	line = 0;
	while ((str = fgets (buf, sizeof (buf), f))) {
		++line;
		eat_whitespace (str);
		if (!str [0])
			continue;
		if (str [0] == '#') {
			g_string_append (comment, str);
			continue;
		}
		p = strchr (str, ':');
		if (!p)
			g_error ("Invalid format at line %d in %s\n", line, name);
		*p++ = 0;
		eat_whitespace (p);
		desc = g_hash_table_lookup (table, str);
		if (!desc)
			g_error ("Invalid opcode '%s' at line %d in %s\n", str, line, name);
		if (desc->desc)
			g_error ("Duplicated opcode '%s' at line %d in %s\n", str, line, name);
		desc->desc = g_strdup (p);
		desc->comment = g_strdup (comment->str);
		g_string_truncate (comment, 0);
		while (*p) {
			if (strncmp (p, "dest:", 5) == 0) {
				desc->spec [MONO_INST_DEST] = p [5];
				p += 6;
			} else if (strncmp (p, "src1:", 5) == 0) {
				desc->spec [MONO_INST_SRC1] = p [5];
				p += 6;
			} else if (strncmp (p, "src2:", 5) == 0) {
				desc->spec [MONO_INST_SRC2] = p [5];
				p += 6;
			} else if (strncmp (p, "cost:", 5) == 0) {
				desc->spec [MONO_INST_COST] = p [5];
				p += 6;
			} else if (strncmp (p, "clob:", 5) == 0) {
				desc->spec [MONO_INST_CLOB] = p [5];
				p += 6;
			} else if (strncmp (p, "res:", 4) == 0) {
				desc->spec [MONO_INST_RES] = p [4];
				p += 5;
			} else if (strncmp (p, "flags:", 6) == 0) {
				desc->spec [MONO_INST_FLAGS] = p [6];
				p += 7;
			} else if (strncmp (p, "delay:", 6) == 0) {
				desc->spec [MONO_INST_DELAY] = p [6];
				p += 7;
			} else if (strncmp (p, "len:", 4) == 0) {
				p += 4;
				desc->spec [MONO_INST_LEN] = strtoul (p, &p, 10);
			} else {
				g_error ("Parse error at '%s' at line %d in %s\n", p, line, name);
			}
			eat_whitespace (p);
		}
	}
	fclose (f);
	return 0;
}

static OpDesc *opcodes = NULL;

static void
init_table (void) {
	int i;
	OpDesc *desc;

	table = g_hash_table_new (g_str_hash, g_str_equal);

	opcodes = g_new0 (OpDesc, OP_LAST);
	for (i = 0; i < MONO_CEE_LAST; ++i) {
		desc = opcodes + i;
		desc->num = i;
		desc->name = mono_inst_name (i);
		g_hash_table_insert (table, (char *)desc->name, desc);
	}
	for (i = OP_LOAD; i < OP_LAST; ++i) {
		desc = opcodes + i;
		desc->num = i;
		desc->name = mono_inst_name (i);
		g_hash_table_insert (table, (char *)desc->name, desc);
	}
}

static void
output_char (FILE *f, char c) {
	if (isalnum (c))
		fprintf (f, "%c", c);
	else
		fprintf (f, "\\x%x\" \"", c);
}

static void
build_table (const char *fname, const char *name) {
	FILE *f;
	int i, j;
	OpDesc *desc;

	if (!(f = fopen (fname, "w")))
		g_error ("Cannot open file '%s'", fname);
	fprintf (f, "/* File automatically generated by genmdesc, don't change */\n\n");
	fprintf (f, "static const char * const %s [OP_LAST] = {\n", name);

	for (i = 0; i < MONO_CEE_LAST; ++i) {
		desc = opcodes + i;
		if (!desc->desc)
			fprintf (f, "\tNULL,\t/* %s */\n", desc->name);
		else {
			fprintf (f, "\t\"");
			for (j = 0; j < MONO_INST_MAX; ++j)
				output_char (f, desc->spec [j]);
			fprintf (f, "\",\t/* %s */\n", desc->name);
		}
	}
	for (i = MONO_CEE_LAST; i < OP_LOAD; ++i) {
		fprintf (f, "\tNULL, /* unassigned */\n");
	}
	for (i = OP_LOAD; i < OP_LAST; ++i) {
		desc = opcodes + i;
		if (!desc->desc)
			fprintf (f, "\tNULL,\t/* %s */\n", desc->name);
		else {
			fprintf (f, "\t\"");
			for (j = 0; j < MONO_INST_MAX; ++j)
				output_char (f, desc->spec [j]);
			fprintf (f, "\",\t/* %s */\n", desc->name);
		}
	}
	fprintf (f, "};\n\n");
	fclose (f);
}

static void
dump (void) {
	int i;
	OpDesc *desc;
	
	for (i = 0; i < MONO_CEE_LAST; ++i) {
		desc = opcodes + i;
		if (desc->comment)
			g_print ("%s", desc->comment);
		if (!desc->desc)
			g_print ("%s:\n", desc->name);
		else {
			g_print ("%s: %s", desc->name, desc->desc);
			if (!strchr (desc->desc, '\n'))
				g_print ("\n");
		}
	}
	for (i = OP_LOAD; i < OP_LAST; ++i) {
		desc = opcodes + i;
		if (!desc->desc)
			g_print ("%s:\n", desc->name);
		else {
			g_print ("%s: %s", desc->name, desc->desc);
			if (!strchr (desc->desc, '\n'))
				g_print ("\n");
		}
	}
}

/*
 * TODO: output the table (possibly merged), in the input format 
 */
int 
main (int argc, char* argv [])
{
	init_table ();
	switch (argc) {
	case 2:
		/* useful to get a new file when some opcodes are added: looses the comments, though */
		load_file (argv [1]);
		dump ();
		break;
	case 4:
		load_file (argv [1]);
		build_table (argv [2], argv [3]);
		break;
	default:
		g_print ("Usage: genmdesc arguments\n");
		g_print ("\tgenmdesc desc             Output to stdout the description file.\n");
		g_print ("\tgenmdesc desc output name Write to output the description in a table named 'name'.\n");
		return 1;
	}
	return 0;
}

