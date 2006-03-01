/*
 * locales.c: Culture-sensitive handling
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *	Mohammad DAMT (mdamt@cdl2000.com)
 *
 * (C) 2003 Ximian, Inc.
 * (C) 2003 PT Cakram Datalingga Duaribu  http://www.cdl2000.com
 */

#include <config.h>
#include <glib.h>
#include <string.h>

#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/monitor.h>
#include <mono/metadata/locales.h>
#include <mono/metadata/culture-info.h>
#include <mono/metadata/culture-info-tables.h>


#include <locale.h>

#undef DEBUG

static gint32 string_invariant_compare_char (gunichar2 c1, gunichar2 c2,
					     gint32 options);
static gint32 string_invariant_compare (MonoString *str1, gint32 off1,
					gint32 len1, MonoString *str2,
					gint32 off2, gint32 len2,
					gint32 options);
static MonoString *string_invariant_replace (MonoString *me,
					     MonoString *oldValue,
					     MonoString *newValue);
static gint32 string_invariant_indexof (MonoString *source, gint32 sindex,
					gint32 count, MonoString *value,
					MonoBoolean first);
static gint32 string_invariant_indexof_char (MonoString *source, gint32 sindex,
					     gint32 count, gunichar2 value,
					     MonoBoolean first);

static const CultureInfoEntry* culture_info_entry_from_lcid (int lcid);

static const RegionInfoEntry* region_info_entry_from_lcid (int lcid);

static int
culture_lcid_locator (const void *a, const void *b)
{
	const CultureInfoEntry *aa = a;
	const CultureInfoEntry *bb = b;

	return (aa->lcid - bb->lcid);
}

static int
region_lcid_locator (const void *a, const void *b)
{
	const int *lcid = a;
	const CultureInfoEntry *bb = b;

	return *lcid - bb->lcid;
}

static int
culture_name_locator (const void *a, const void *b)
{
	const char *aa = a;
	const CultureInfoNameEntry *bb = b;
	int ret;
	
	ret = strcmp (aa, idx2string (bb->name));

	return ret;
}

static int
region_name_locator (const void *a, const void *b)
{
	const char *aa = a;
	const RegionInfoNameEntry *bb = b;
	int ret;
	
	ret = strcmp (aa, idx2string (bb->name));

	return ret;
}

static MonoArray*
create_group_sizes_array (const gint *gs, gint ml)
{
	MonoArray *ret;
	int i, len = 0;

	for (i = 0; i < ml; i++) {
		if (gs [i] == -1)
			break;
		len++;
	}
	
	ret = mono_array_new (mono_domain_get (),
			mono_get_int32_class (), len);

	for(i = 0; i < len; i++)
		mono_array_set (ret, gint32, i, gs [i]);

	return ret;
}

static MonoArray*
create_names_array_idx (const guint16 *names, int ml)
{
	MonoArray *ret;
	MonoDomain *domain;
	int i, len = 0;

	if (names == NULL)
		return NULL;

	domain = mono_domain_get ();

	for (i = 0; i < ml; i++) {
		if (names [i] == 0)
			break;
		len++;
	}

	ret = mono_array_new (mono_domain_get (), mono_get_string_class (), len);

	for(i = 0; i < len; i++)
		mono_array_setref (ret, i, mono_string_new (domain, idx2string (names [i])));

	return ret;
}

void
ves_icall_System_Globalization_CultureInfo_construct_datetime_format (MonoCultureInfo *this)
{
	MonoDomain *domain;
	MonoDateTimeFormatInfo *datetime;
	const DateTimeFormatEntry *dfe;

	MONO_ARCH_SAVE_REGS;

	g_assert (this->datetime_index >= 0);

	datetime = this->datetime_format;
	dfe = &datetime_format_entries [this->datetime_index];

	domain = mono_domain_get ();

	datetime->AbbreviatedDayNames = create_names_array_idx (dfe->abbreviated_day_names,
			NUM_DAYS);
	datetime->AbbreviatedMonthNames = create_names_array_idx (dfe->abbreviated_month_names,
			NUM_MONTHS);
	datetime->AMDesignator = mono_string_new (domain, idx2string (dfe->am_designator));
	datetime->CalendarWeekRule = dfe->calendar_week_rule;
	datetime->DateSeparator = mono_string_new (domain, idx2string (dfe->date_separator));
	datetime->DayNames = create_names_array_idx (dfe->day_names, NUM_DAYS);
	datetime->FirstDayOfWeek = dfe->first_day_of_week;
	datetime->FullDateTimePattern = mono_string_new (domain, idx2string (dfe->full_date_time_pattern));
	datetime->LongDatePattern = mono_string_new (domain, idx2string (dfe->long_date_pattern));
	datetime->LongTimePattern = mono_string_new (domain, idx2string (dfe->long_time_pattern));
	datetime->MonthDayPattern = mono_string_new (domain, idx2string (dfe->month_day_pattern));
	datetime->MonthNames = create_names_array_idx (dfe->month_names, NUM_MONTHS);
	datetime->PMDesignator = mono_string_new (domain, idx2string (dfe->pm_designator));
	datetime->ShortDatePattern = mono_string_new (domain, idx2string (dfe->short_date_pattern));
	datetime->ShortTimePattern = mono_string_new (domain, idx2string (dfe->short_time_pattern));
	datetime->TimeSeparator = mono_string_new (domain, idx2string (dfe->time_separator));
	datetime->YearMonthPattern = mono_string_new (domain, idx2string (dfe->year_month_pattern));
	datetime->ShortDatePatterns = create_names_array_idx (dfe->short_date_patterns,
			NUM_SHORT_DATE_PATTERNS);
	datetime->LongDatePatterns = create_names_array_idx (dfe->long_date_patterns,
			NUM_LONG_DATE_PATTERNS);
	datetime->ShortTimePatterns = create_names_array_idx (dfe->short_time_patterns,
			NUM_SHORT_TIME_PATTERNS);
	datetime->LongTimePatterns = create_names_array_idx (dfe->long_time_patterns,
			NUM_LONG_TIME_PATTERNS);

}

void
ves_icall_System_Globalization_CultureInfo_construct_number_format (MonoCultureInfo *this)
{
	MonoDomain *domain;
	MonoNumberFormatInfo *number;
	const NumberFormatEntry *nfe;

	MONO_ARCH_SAVE_REGS;

	g_assert (this->number_format != 0);

	number = this->number_format;
	nfe = &number_format_entries [this->number_index];

	domain = mono_domain_get ();

	number->currencyDecimalDigits = nfe->currency_decimal_digits;
	number->currencyDecimalSeparator = mono_string_new (domain,
			idx2string (nfe->currency_decimal_separator));
	number->currencyGroupSeparator = mono_string_new (domain,
			idx2string (nfe->currency_group_separator));
	number->currencyGroupSizes = create_group_sizes_array (nfe->currency_group_sizes,
			GROUP_SIZE);
	number->currencyNegativePattern = nfe->currency_negative_pattern;
	number->currencyPositivePattern = nfe->currency_positive_pattern;
	number->currencySymbol = mono_string_new (domain, idx2string (nfe->currency_symbol));
	number->naNSymbol = mono_string_new (domain, idx2string (nfe->nan_symbol));
	number->negativeInfinitySymbol = mono_string_new (domain,
			idx2string (nfe->negative_infinity_symbol));
	number->negativeSign = mono_string_new (domain, idx2string (nfe->negative_sign));
	number->numberDecimalDigits = nfe->number_decimal_digits;
	number->numberDecimalSeparator = mono_string_new (domain,
			idx2string (nfe->number_decimal_separator));
	number->numberGroupSeparator = mono_string_new (domain, idx2string (nfe->number_group_separator));
	number->numberGroupSizes = create_group_sizes_array (nfe->number_group_sizes,
			GROUP_SIZE);
	number->numberNegativePattern = nfe->number_negative_pattern;
	number->percentDecimalDigits = nfe->percent_decimal_digits;
	number->percentDecimalSeparator = mono_string_new (domain,
			idx2string (nfe->percent_decimal_separator));
	number->percentGroupSeparator = mono_string_new (domain,
			idx2string (nfe->percent_group_separator));
	number->percentGroupSizes = create_group_sizes_array (nfe->percent_group_sizes,
			GROUP_SIZE);
	number->percentNegativePattern = nfe->percent_negative_pattern;
	number->percentPositivePattern = nfe->percent_positive_pattern;
	number->percentSymbol = mono_string_new (domain, idx2string (nfe->percent_symbol));
	number->perMilleSymbol = mono_string_new (domain, idx2string (nfe->per_mille_symbol));
	number->positiveInfinitySymbol = mono_string_new (domain,
			idx2string (nfe->positive_infinity_symbol));
	number->positiveSign = mono_string_new (domain, idx2string (nfe->positive_sign));
}

static MonoBoolean
construct_culture (MonoCultureInfo *this, const CultureInfoEntry *ci)
{
	MonoDomain *domain = mono_domain_get ();

	this->lcid = ci->lcid;
	this->name = mono_string_new (domain, idx2string (ci->name));
	this->icu_name = mono_string_new (domain, idx2string (ci->icu_name));
	this->displayname = mono_string_new (domain, idx2string (ci->displayname));
	this->englishname = mono_string_new (domain, idx2string (ci->englishname));
	this->nativename = mono_string_new (domain, idx2string (ci->nativename));
	this->win3lang = mono_string_new (domain, idx2string (ci->win3lang));
	this->iso3lang = mono_string_new (domain, idx2string (ci->iso3lang));
	this->iso2lang = mono_string_new (domain, idx2string (ci->iso2lang));
	this->parent_lcid = ci->parent_lcid;
	this->specific_lcid = ci->specific_lcid;
	this->datetime_index = ci->datetime_format_index;
	this->number_index = ci->number_format_index;
	this->calendar_data = ci->calendar_data;
	this->text_info_data = &ci->text_info;
	
	return TRUE;
}

static MonoBoolean
construct_region (MonoRegionInfo *this, const RegionInfoEntry *ri)
{
	MonoDomain *domain = mono_domain_get ();

	this->region_id = ri->region_id;
	this->iso2name = mono_string_new (domain, idx2string (ri->iso2name));
	this->iso3name = mono_string_new (domain, idx2string (ri->iso3name));
	this->win3name = mono_string_new (domain, idx2string (ri->win3name));
	this->english_name = mono_string_new (domain, idx2string (ri->english_name));
	this->currency_symbol = mono_string_new (domain, idx2string (ri->currency_symbol));
	this->iso_currency_symbol = mono_string_new (domain, idx2string (ri->iso_currency_symbol));
	this->currency_english_name = mono_string_new (domain, idx2string (ri->currency_english_name));
	
	return TRUE;
}

static gboolean
construct_culture_from_specific_name (MonoCultureInfo *ci, gchar *name)
{
	const CultureInfoEntry *entry;
	const CultureInfoNameEntry *ne;

	MONO_ARCH_SAVE_REGS;

	ne = bsearch (name, culture_name_entries, NUM_CULTURE_ENTRIES,
			sizeof (CultureInfoNameEntry), culture_name_locator);

	if (ne == NULL)
		return FALSE;

	entry = &culture_entries [ne->culture_entry_index];

	/* try avoiding another lookup, often the culture is its own specific culture */
	if (entry->lcid != entry->specific_lcid)
		entry = culture_info_entry_from_lcid (entry->specific_lcid);

	return construct_culture (ci, entry);
}

static gboolean
construct_region_from_specific_name (MonoRegionInfo *ri, gchar *name)
{
	const RegionInfoEntry *entry;
	const RegionInfoNameEntry *ne;

	MONO_ARCH_SAVE_REGS;

	ne = bsearch (name, region_name_entries, NUM_REGION_ENTRIES,
			sizeof (RegionInfoNameEntry), region_name_locator);

	if (ne == NULL)
		return FALSE;

	entry = &region_entries [ne->region_entry_index];

	return construct_region (ri, entry);
}

static const CultureInfoEntry*
culture_info_entry_from_lcid (int lcid)
{
	const CultureInfoEntry *ci;
	CultureInfoEntry key;

	key.lcid = lcid;
	ci = bsearch (&key, culture_entries, NUM_CULTURE_ENTRIES, sizeof (CultureInfoEntry), culture_lcid_locator);

	return ci;
}

static const RegionInfoEntry*
region_info_entry_from_lcid (int lcid)
{
	const RegionInfoEntry *entry;
	const CultureInfoEntry *ne;

	MONO_ARCH_SAVE_REGS;

	ne = bsearch (&lcid, culture_entries, NUM_CULTURE_ENTRIES,
			sizeof (CultureInfoEntry), region_lcid_locator);

	if (ne == NULL)
		return FALSE;

	entry = &region_entries [ne->region_entry_index];

	return entry;
}

/*
 * The following two methods are modified from the ICU source code. (http://oss.software.ibm.com/icu)
 * Copyright (c) 1995-2003 International Business Machines Corporation and others
 * All rights reserved.
 */
static gchar*
get_posix_locale (void)
{
	const gchar* posix_locale = NULL;

	posix_locale = g_getenv("LC_ALL");
	if (posix_locale == 0) {
		posix_locale = g_getenv("LANG");
		if (posix_locale == 0) {
			posix_locale = setlocale(LC_ALL, NULL);
		}
	}

	if (posix_locale == NULL)
		return NULL;

	if ((strcmp ("C", posix_locale) == 0) || (strchr (posix_locale, ' ') != NULL)
			|| (strchr (posix_locale, '/') != NULL)) {
		/*
		 * HPUX returns 'C C C C C C C'
		 * Solaris can return /en_US/C/C/C/C/C on the second try.
		 * Maybe we got some garbage.
		 */
		return NULL;
	}

	return g_strdup (posix_locale);
}

static gchar*
get_current_locale_name (void)
{
	gchar *locale;
	gchar *corrected = NULL;
	const gchar *p;
        gchar *c;

#ifdef PLATFORM_WIN32
	locale = g_win32_getlocale ();
#else	
	locale = get_posix_locale ();
#endif	

	if (locale == NULL)
		return NULL;

	if ((p = strchr (locale, '.')) != NULL) {
		/* assume new locale can't be larger than old one? */
		corrected = malloc (strlen (locale));
		strncpy (corrected, locale, p - locale);
		corrected [p - locale] = 0;

		/* do not copy after the @ */
		if ((p = strchr (corrected, '@')) != NULL)
			corrected [p - corrected] = 0;
	}

	/* Note that we scan the *uncorrected* ID. */
	if ((p = strrchr (locale, '@')) != NULL) {

		/*
		 * In Mono we dont handle the '@' modifier because we do
		 * not have any cultures that use it. We just trim it
		 * off of the end of the name.
		 */

		if (corrected == NULL) {
			corrected = malloc (strlen (locale));
			strncpy (corrected, locale, p - locale);
			corrected [p - locale] = 0;
		}
	}

	if (corrected == NULL)
		corrected = locale;
	else
		g_free (locale);

	if ((c = strchr (corrected, '_')) != NULL)
		*c = '-';

	g_strdown (corrected);

	return corrected;
}	 

MonoBoolean
ves_icall_System_Globalization_CultureInfo_construct_internal_locale_from_current_locale (MonoCultureInfo *ci)
{
	gchar *locale;
	gboolean ret;

	MONO_ARCH_SAVE_REGS;

	locale = get_current_locale_name ();
	if (locale == NULL)
		return FALSE;

	ret = construct_culture_from_specific_name (ci, locale);
	g_free (locale);

	return ret;
}

MonoBoolean
ves_icall_System_Globalization_CultureInfo_construct_internal_locale_from_lcid (MonoCultureInfo *this,
		gint lcid)
{
	const CultureInfoEntry *ci;
	
	MONO_ARCH_SAVE_REGS;

	ci = culture_info_entry_from_lcid (lcid);
	if(ci == NULL)
		return FALSE;

	return construct_culture (this, ci);
}

MonoBoolean
ves_icall_System_Globalization_CultureInfo_construct_internal_locale_from_name (MonoCultureInfo *this,
		MonoString *name)
{
	const CultureInfoNameEntry *ne;
	char *n;
	
	MONO_ARCH_SAVE_REGS;

	n = mono_string_to_utf8 (name);
	ne = bsearch (n, culture_name_entries, NUM_CULTURE_ENTRIES,
			sizeof (CultureInfoNameEntry), culture_name_locator);

	if (ne == NULL) {
		/*g_print ("ne (%s) is null\n", n);*/
		g_free (n);
		return FALSE;
	}
	g_free (n);

	return construct_culture (this, &culture_entries [ne->culture_entry_index]);
}

MonoBoolean
ves_icall_System_Globalization_CultureInfo_construct_internal_locale_from_specific_name (MonoCultureInfo *ci,
		MonoString *name)
{
	gchar *locale;
	gboolean ret;

	MONO_ARCH_SAVE_REGS;

	locale = mono_string_to_utf8 (name);
	ret = construct_culture_from_specific_name (ci, locale);
	g_free (locale);

	return ret;
}

MonoBoolean
ves_icall_System_Globalization_RegionInfo_construct_internal_region_from_lcid (MonoRegionInfo *this,
		gint lcid)
{
	const RegionInfoEntry *ri;
	
	MONO_ARCH_SAVE_REGS;

	ri = region_info_entry_from_lcid (lcid);
	if(ri == NULL)
		return FALSE;

	return construct_region (this, ri);
}

MonoBoolean
ves_icall_System_Globalization_RegionInfo_construct_internal_region_from_name (MonoRegionInfo *this,
		MonoString *name)
{
	const RegionInfoNameEntry *ne;
	char *n;
	
	MONO_ARCH_SAVE_REGS;

	n = mono_string_to_utf8 (name);
	ne = bsearch (n, region_name_entries, NUM_REGION_ENTRIES,
		sizeof (RegionInfoNameEntry), region_name_locator);

	if (ne == NULL) {
		/*g_print ("ne (%s) is null\n", n);*/
		g_free (n);
		return FALSE;
	}
	g_free (n);

	return construct_region (this, &region_entries [ne->region_entry_index]);
}

MonoArray*
ves_icall_System_Globalization_CultureInfo_internal_get_cultures (MonoBoolean neutral,
		MonoBoolean specific, MonoBoolean installed)
{
	MonoArray *ret;
	MonoClass *class;
	MonoCultureInfo *culture;
	MonoDomain *domain;
	const CultureInfoEntry *ci;
	gint i, len;
	gboolean is_neutral;

	MONO_ARCH_SAVE_REGS;

	domain = mono_domain_get ();

	len = 0;
	for (i = 0; i < NUM_CULTURE_ENTRIES; i++) {
		ci = &culture_entries [i];
		is_neutral = ((ci->lcid & 0xff00) == 0 || ci->specific_lcid == 0);
		if ((neutral && is_neutral) || (specific && !is_neutral))
			len++;
	}

	class = mono_class_from_name (mono_get_corlib (),
			"System.Globalization", "CultureInfo");

	/* The InvariantCulture is not in culture_entries */
	/* We reserve the first slot in the array for it */
	if (neutral)
		len++;

	ret = mono_array_new (domain, class, len);

	if (len == 0)
		return ret;

	len = 0;
	if (neutral)
		mono_array_setref (ret, len++, NULL);

	for (i = 0; i < NUM_CULTURE_ENTRIES; i++) {
		ci = &culture_entries [i];
		is_neutral = ((ci->lcid & 0xff00) == 0 || ci->specific_lcid == 0);
		if ((neutral && is_neutral) || (specific && !is_neutral)) {
			culture = (MonoCultureInfo *) mono_object_new (domain, class);
			mono_runtime_object_init ((MonoObject *) culture);
			construct_culture (culture, ci);
			mono_array_setref (ret, len++, culture);
		}
	}

	return ret;
}

/**
 * ves_icall_System_Globalization_CultureInfo_internal_is_lcid_neutral:
 * 
 * Set is_neutral and return TRUE if the culture is found. If it is not found return FALSE.
 */
MonoBoolean
ves_icall_System_Globalization_CultureInfo_internal_is_lcid_neutral (gint lcid, MonoBoolean *is_neutral)
{
	const CultureInfoEntry *entry;

	MONO_ARCH_SAVE_REGS;

	entry = culture_info_entry_from_lcid (lcid);

	if (entry == NULL)
		return FALSE;

	*is_neutral = (entry->specific_lcid == 0);

	return TRUE;
}

void ves_icall_System_Globalization_CultureInfo_construct_internal_locale (MonoCultureInfo *this, MonoString *locale)
{
	MONO_ARCH_SAVE_REGS;
	
	/* Always claim "unknown locale" if we don't have ICU (only
	 * called for non-invariant locales)
	 */
	mono_raise_exception((MonoException *)mono_exception_from_name(mono_get_corlib (), "System", "ArgumentException"));
}

void ves_icall_System_Globalization_CompareInfo_construct_compareinfo (MonoCompareInfo *comp, MonoString *locale)
{
	/* Nothing to do here */
}

int ves_icall_System_Globalization_CompareInfo_internal_compare (MonoCompareInfo *this, MonoString *str1, gint32 off1, gint32 len1, MonoString *str2, gint32 off2, gint32 len2, gint32 options)
{
	MONO_ARCH_SAVE_REGS;
	
	/* Do a normal ascii string compare, as we only know the
	 * invariant locale if we dont have ICU
	 */
	return(string_invariant_compare (str1, off1, len1, str2, off2, len2,
					 options));
}

void ves_icall_System_Globalization_CompareInfo_free_internal_collator (MonoCompareInfo *this)
{
	/* Nothing to do here */
}

void ves_icall_System_Globalization_CompareInfo_assign_sortkey (MonoCompareInfo *this, MonoSortKey *key, MonoString *source, gint32 options)
{
	MonoArray *arr;
	gint32 keylen, i;

	MONO_ARCH_SAVE_REGS;
	
	keylen=mono_string_length (source);
	
	arr=mono_array_new (mono_domain_get (), mono_get_byte_class (),
			    keylen);
	for(i=0; i<keylen; i++) {
		mono_array_set (arr, guint8, i, mono_string_chars (source)[i]);
	}
	
	key->key=arr;
}

int ves_icall_System_Globalization_CompareInfo_internal_index (MonoCompareInfo *this, MonoString *source, gint32 sindex, gint32 count, MonoString *value, gint32 options, MonoBoolean first)
{
	MONO_ARCH_SAVE_REGS;
	
	return(string_invariant_indexof (source, sindex, count, value, first));
}

int ves_icall_System_Globalization_CompareInfo_internal_index_char (MonoCompareInfo *this, MonoString *source, gint32 sindex, gint32 count, gunichar2 value, gint32 options, MonoBoolean first)
{
	MONO_ARCH_SAVE_REGS;
	
	return(string_invariant_indexof_char (source, sindex, count, value,
					      first));
}

int ves_icall_System_Threading_Thread_current_lcid (void)
{
	MONO_ARCH_SAVE_REGS;
	
	/* Invariant */
	return(0x007F);
}

MonoString *ves_icall_System_String_InternalReplace_Str_Comp (MonoString *this, MonoString *old, MonoString *new, MonoCompareInfo *comp)
{
	MONO_ARCH_SAVE_REGS;
	
	/* Do a normal ascii string compare and replace, as we only
	 * know the invariant locale if we dont have ICU
	 */
	return(string_invariant_replace (this, old, new));
}

static gint32 string_invariant_compare_char (gunichar2 c1, gunichar2 c2,
					     gint32 options)
{
	gint32 result;
	GUnicodeType c1type, c2type;

	/* Ordinal can not be mixed with other options, and must return the difference, not only -1, 0, 1 */
	if (options & CompareOptions_Ordinal) 
		return (gint32) c1 - c2;
	
	c1type = g_unichar_type (c1);
	c2type = g_unichar_type (c2);
	
	if (options & CompareOptions_IgnoreCase) {
		result = (gint32) (c1type != G_UNICODE_LOWERCASE_LETTER ? g_unichar_tolower(c1) : c1) -
			(c2type != G_UNICODE_LOWERCASE_LETTER ? g_unichar_tolower(c2) : c2);
	} else {
		/*
		 * No options. Kana, symbol and spacing options don't
		 * apply to the invariant culture.
		 */

		/*
		 * FIXME: here we must use the information from c1type and c2type
		 * to find out the proper collation, even on the InvariantCulture, the
		 * sorting is not done by computing the unicode values, but their
		 * actual sort order.
		 */
		result = (gint32) c1 - c2;
	}

	return ((result < 0) ? -1 : (result > 0) ? 1 : 0);
}

static gint32 string_invariant_compare (MonoString *str1, gint32 off1,
					gint32 len1, MonoString *str2,
					gint32 off2, gint32 len2,
					gint32 options)
{
	/* c translation of C# code from old string.cs.. :) */
	gint32 length;
	gint32 charcmp;
	gunichar2 *ustr1;
	gunichar2 *ustr2;
	gint32 pos;

	if(len1 >= len2) {
		length=len1;
	} else {
		length=len2;
	}

	ustr1 = mono_string_chars(str1)+off1;
	ustr2 = mono_string_chars(str2)+off2;

	pos = 0;

	for (pos = 0; pos != length; pos++) {
		if (pos >= len1 || pos >= len2)
			break;

		charcmp = string_invariant_compare_char(ustr1[pos], ustr2[pos],
							options);
		if (charcmp != 0) {
			return(charcmp);
		}
	}

	/* the lesser wins, so if we have looped until length we just
	 * need to check the last char
	 */
	if (pos == length) {
		return(string_invariant_compare_char(ustr1[pos - 1],
						     ustr2[pos - 1], options));
	}

	/* Test if one of the strings has been compared to the end */
	if (pos >= len1) {
		if (pos >= len2) {
			return(0);
		} else {
			return(-1);
		}
	} else if (pos >= len2) {
		return(1);
	}

	/* if not, check our last char only.. (can this happen?) */
	return(string_invariant_compare_char(ustr1[pos], ustr2[pos], options));
}

static MonoString *string_invariant_replace (MonoString *me,
					     MonoString *oldValue,
					     MonoString *newValue)
{
	MonoString *ret;
	gunichar2 *src;
	gunichar2 *dest=NULL; /* shut gcc up */
	gunichar2 *oldstr;
	gunichar2 *newstr=NULL; /* shut gcc up here too */
	gint32 i, destpos;
	gint32 occurr;
	gint32 newsize;
	gint32 oldstrlen;
	gint32 newstrlen;
	gint32 srclen;

	occurr = 0;
	destpos = 0;

	oldstr = mono_string_chars(oldValue);
	oldstrlen = mono_string_length(oldValue);

	if (NULL != newValue) {
		newstr = mono_string_chars(newValue);
		newstrlen = mono_string_length(newValue);
	} else
		newstrlen = 0;

	src = mono_string_chars(me);
	srclen = mono_string_length(me);

	if (oldstrlen != newstrlen) {
		i = 0;
		while (i <= srclen - oldstrlen) {
			if (0 == memcmp(src + i, oldstr, oldstrlen * sizeof(gunichar2))) {
				occurr++;
				i += oldstrlen;
			}
			else
				i ++;
		}
		if (occurr == 0)
			return me;
		newsize = srclen + ((newstrlen - oldstrlen) * occurr);
	} else
		newsize = srclen;

	ret = NULL;
	i = 0;
	while (i < srclen) {
		if (0 == memcmp(src + i, oldstr, oldstrlen * sizeof(gunichar2))) {
			if (ret == NULL) {
				ret = mono_string_new_size( mono_domain_get (), newsize);
				dest = mono_string_chars(ret);
				memcpy (dest, src, i * sizeof(gunichar2));
			}
			if (newstrlen > 0) {
				memcpy(dest + destpos, newstr, newstrlen * sizeof(gunichar2));
				destpos += newstrlen;
			}
			i += oldstrlen;
			continue;
		} else if (ret != NULL) {
			dest[destpos] = src[i];
		}
		destpos++;
		i++;
	}
	
	if (ret == NULL)
		return me;

	return ret;
}

static gint32 string_invariant_indexof (MonoString *source, gint32 sindex,
					gint32 count, MonoString *value,
					MonoBoolean first)
{
	gint32 lencmpstr;
	gunichar2 *src;
	gunichar2 *cmpstr;
	gint32 pos,i;
	
	lencmpstr = mono_string_length(value);
	
	src = mono_string_chars(source);
	cmpstr = mono_string_chars(value);

	if(first) {
		count -= lencmpstr;
		for(pos=sindex;pos <= sindex+count;pos++) {
			for(i=0;src[pos+i]==cmpstr[i];) {
				if(++i==lencmpstr) {
					return(pos);
				}
			}
		}
		
		return(-1);
	} else {
		for(pos=sindex-lencmpstr+1;pos>sindex-count;pos--) {
			if(memcmp (src+pos, cmpstr,
				   lencmpstr*sizeof(gunichar2))==0) {
				return(pos);
			}
		}
		
		return(-1);
	}
}

static gint32 string_invariant_indexof_char (MonoString *source, gint32 sindex,
					     gint32 count, gunichar2 value,
					     MonoBoolean first)
{
	gint32 pos;
	gunichar2 *src;

	src = mono_string_chars(source);
	if(first) {
		for (pos = sindex; pos != count + sindex; pos++) {
			if (src [pos] == value) {
				return(pos);
			}
		}

		return(-1);
	} else {
		for (pos = sindex; pos > sindex - count; pos--) {
			if (src [pos] == value)
				return(pos);
		}

		return(-1);
	}
}

