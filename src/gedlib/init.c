/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=============================================================
 * init.c -- Initialize LifeLines data structures
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 05 Oct 94    3.0.2 - 09 Nov 94
 *   3.0.3 - 21 Sep 95
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "arch.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "version.h"
#include "lloptions.h"
#include "codesets.h"
#include "menuitem.h"
#include "zstr.h"
#include "icvt.h"
#include "date.h"
#include "mychar.h"
#include "charprops.h"
#include "xlat.h"
#include "dbcontext.h"

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable=NULL;		/* table for tag strings */
TABLE placabbvs=NULL;	/* table for place abbrevs */
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN writeable;
extern STRING readpath,readpath_file;
extern STRING illegal_char;
extern INT opt_finnish, opt_mychar;

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN load_configs(STRING configfile, STRING * pmsg);
static BOOLEAN open_database_impl(LLDATABASE lldb, INT alteration);
static void post_codesets_hook(void);
static void pre_codesets_hook(void);
static void update_db_options(void);

/*********************************************
 * local variables
 *********************************************/

static BOOLEAN suppress_reload=FALSE;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================
 * init_lifelines_global -- Initialization options & misc. stuff
 *  This is called before first (or later) database opened
 *===============================*/
BOOLEAN
init_lifelines_global (STRING configfile, STRING * pmsg, void (*notify)(STRING db, BOOLEAN opening))
{
	STRING e;
	STRING dirvars[] = { "LLPROGRAMS", "LLREPORTS", "LLARCHIVES"
		, "LLDATABASES", };
	INT i;

	/* request notification when options change */
	register_notify(&update_useropts);
	suppress_reload = TRUE;

	dbnotify_set(notify);

	if (!load_configs(configfile, pmsg)) {
		suppress_reload = FALSE;
		update_useropts(NULL);
		return FALSE;
	}

	pre_codesets_hook(); /* For MS-Windows user config of console codepages */

	/* now that codeset variables are set from config file, lets initialize codesets */
	/* although int_codeset can't be determined yet, we need GUI codeset for gettext */
	init_codesets();

	post_codesets_hook(); /* For Windows, link dynamically to gettext & iconv if available */


	/* until we have an internal codeset (which is until we open a database)
	we want output in display codeset */
	llgettext_init(PACKAGE, gui_codeset_out);

	/* read available translation tables */
	transl_load_all_tts();
	/* set up translations (for first time, will do it again after 
	loading config table, and then again after loading database */
	transl_load_xlats();

	/* check if any directories not specified, and try environment
	variables, and default to "." */
	for (i=0; i<ARRSIZE(dirvars); ++i) {
		STRING str = getenv(dirvars[i]);
		if (!str)
			str = ".";
		setoptstr_fallback(dirvars[i], str);
	}
	/* also check environment variable for editor */
	{
		STRING str = getenv("LLEDITOR");
		if (!str)
			str = environ_determine_editor(PROGRAM_LIFELINES);
		setoptstr_fallback("LLEDITOR", str);
	}
	/* editor falls back to platform-specific default */
	e = getlloptstr("LLEDITOR", NULL);
	/* configure tempfile & edit command */
	editfile = environ_determine_tempfile();
	if (!editfile) {
		*pmsg = strsave("Error creating temp file");
		return FALSE;
	}
	editfile = strsave(editfile );
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	set_usersort(custom_sort);
	suppress_reload = FALSE;
	update_useropts(0);

	/* Finnish always uses custom character sets */
	if (opt_finnish ) {
		opt_mychar = TRUE;
		mych_set_table(ISO_Latin1);
	}
	
	
	return TRUE;
}
/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
BOOLEAN
init_lifelines_db (void)
{
	STRING emsg;
	TABLE dbopts = create_table_str();

	tagtable = create_table_str(); /* values are same as keys */
	placabbvs = create_table_str();

	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", dbopts, '=', &emsg);
	set_db_options(dbopts);
	release_table(dbopts);
	init_caches();
	init_browse_lists();
	if (!openxref(readonly))
		return FALSE;


	transl_load_xlats();

	return TRUE;
}
/*===============================================
 * get_lifelines_version -- Return version string
 *  returns static buffer
 *=============================================*/
STRING
get_lifelines_version (INT maxlen)
{
	static char version[48];
	INT len=sizeof(version);
	if (len>maxlen)
		len=maxlen;
	llstrncpyf(version, len, 0, LIFELINES_VERSION);
	return version;
}
/*===================================
 * close_lifelines -- Close LifeLines
 *  Close entire lifelines engine - not just
 *  database (see close_lldb below).
 *=================================*/
void
close_lifelines (void)
{
	lldb_close(&def_lldb); /* make sure database closed */
	if (editfile) {
		unlink(editfile);
		stdfree(editfile);
		editfile=NULL;
	}
	if (editstr) {
		stdfree(editstr);
		editstr=NULL;
	}
	term_lloptions();
	term_date();
	term_codesets();
	strfree(&int_codeset);
	xlat_shutdown();
}
/*===================================================
 * is_codeset_utf8 -- Is this the name of UTF-8 ?
 *=================================================*/
BOOLEAN
is_codeset_utf8 (CNSTRING codename)
{
	if (!codename || !codename[0]) return FALSE;
	if (eqstr("UTF-8", codename)||eqstr("utf-8", codename)||eqstr("65001", codename))
		return TRUE;
	return FALSE;
}
/*===================================================
 * update_useropts -- Set any global variables
 * dependent on user options
 *=================================================*/
void
update_useropts (VPTR uparm)
{
	uparm = uparm; /* unused */
	if (suppress_reload)
		return;
	/* deal with db-specific options */
	/* includes setting int_codeset */
	if (def_lldb)
		update_db_options();
	/* in case user changed any codesets */
	init_codesets();
	/* in case user changed locale (need int_codeset already set) */
	uilocale();
	/* in case user changed codesets */
	/* TODO: Isn't this superfluous, as it was called in update_db_options above ? */
	transl_load_xlats();

	strupdate(&illegal_char, getlloptstr("IllegalChar", 0));

	nodechk_enable(!!getlloptint("nodecheck", 0));
}
/*==================================================
 * update_db_options -- 
 *  check database-specific options for updates
 *================================================*/
static void
update_db_options (void)
{
	TABLE opttab = create_table_str();
	CNSTRING str=0;
	get_db_options(opttab);

	str = valueof_str(opttab, "codeset");
	if (!str || !str[0]) {
		/*
		no specified database/internal codeset
		so default to user's default codeset, which
		should be from locale
		*/
		str = get_defcodeset();
	}
	if (!int_codeset)
		strupdate(&int_codeset, "");
	if (!eqstr_ex(int_codeset, str)) {
		strupdate(&int_codeset, str);
		uu8 = is_codeset_utf8(int_codeset);
		/* always translate to internal codeset */
		set_gettext_codeset(PACKAGE, int_codeset);
		/* need to reload all predefined codeset conversions */
		transl_load_xlats();
		if (uu8) {
			charprops_load_utf8();
		} else {
			charprops_load(int_codeset);
		}
	}

	destroy_table(opttab);
}
/*==================================================
 * pre_codesets_hook -- code to run just before initializing codesets
 * For MS-Windows user config of console codepages
 *================================================*/
static void
pre_codesets_hook (void)
{
#ifdef WIN32
	/* On MS-Windows, attempt to set any requested non-standard codepage */
	/* Do this now, before init_codesets below */
	INT i = getlloptint("ConsoleCodepage", 0);
	if (i) {
		w_set_oemout_codepage(i);
		w_set_oemin_codepage(i);
	}
#endif
}
/*==================================================
 * post_codesets_hook -- code to run just after initializing codesets
 * For Windows, link dynamically to gettext & iconv if available
 *================================================*/
static void
post_codesets_hook (void)
{
	init_win32_gettext_shim();
	init_win32_iconv_shim(getlloptstr("iconv.path",""));
}
/*==================================================
 * load_configs -- Load global config file(s)
 * returns FALSE if error, with message in pmsg
 *================================================*/
static BOOLEAN
load_configs (STRING configfile, STRING * pmsg)
{
	INT rtn=0;
	STRING str=0;

	/* TODO: Should read a system-wide config file */

	if (!configfile)
		configfile = getenv("LLCONFIGFILE");

	*pmsg = NULL;


	if (configfile && configfile[0]) {

		rtn = load_global_options(configfile, pmsg);
		if (rtn == -1) return FALSE;

	} else {

		/* No config file specified, so try local config_file */
		STRING cfg_file = environ_determine_config_file();

		rtn = load_global_options(cfg_file, pmsg);
		if (rtn == -1) return FALSE;
		if (rtn == 0) {

			/* No config file found, so try $HOME/config_file */
			char cfg_name[MAXPATHLEN];
			/* TODO: Shouldn't Win32 use getenv("USERPROFILE") ? */
			llstrncpy(cfg_name, getenv("HOME") , sizeof(cfg_name), 0);
			llstrappc(cfg_name, sizeof(cfg_name), '/');
			llstrapps(cfg_name, sizeof(cfg_name), 0, cfg_file);

			rtn = load_global_options(cfg_name, pmsg);
			if (rtn == -1) return FALSE;

		}
	}
	if (rtn == 0) return TRUE; /* no config file found */

	/* allow chaining to one more config file */

	str = getlloptstr("LLCONFIGFILE", NULL);
	if (str && str[0]) {
		rtn = load_global_options(str, pmsg);
		if (rtn == -1) return FALSE;
	}
	return TRUE;
}
