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
#include "sys_inc.h"
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include "llstdlib.h"
#include "bfs.h"
#include "icvt.h"

extern int opt_finnish;

static usersortfnc usersort = 0;

static BOOLEAN widecmp(char *str1, char *str2, INT *rtn);

/*===================================================
 * ll_strcmp -- Compare two strings
 * currently handles Finnish build (hard-coded compare)
 * locale build
 * and simple strcmp
 * Perry 2001/07/21 moved all custom sort code to
 * llstrcmploc, which is only used via cmpstrloc
 * Most callers of this had been using eqstr, and did NOT
 * need custom sort (eg, eqstr(tag,"HEAD"))
 *=================================================*/
#if UNUSED_CODE
int
ll_strcmp (char *str1, char *str2)
{
	return strcmp(str1, str2);
}
#endif
/*===================================================
 * ll_strcmploc -- Compare two strings with locale
 * which is to say, either Finnish build (hard-coded compare)
 * or user-specified custom sort
 * or locale info if available (HAVE_STRCOLL)
 * Created: 2001/07/21 (Perry Rapp)
 *=================================================*/
int
ll_strcmploc (char *str1, char *str2)
{
	INT rtn;
	if (opt_finnish)
		return(MY_STRCMP(str1, str2));
	if (usersort && (*usersort)(str1, str2, &rtn))
		return rtn;

	if (widecmp(str1, str2, &rtn))
		return rtn;

#ifdef HAVE_STRCOLL
	errno = 0;
	rtn = strcoll(str1, str2); /* sets errno if fails */
	return !errno ? rtn : strcmp(str1, str2);
#else
	return(strcmp(str1, str2));
#endif
}
/*===================================================
 * ll_strncmp -- Compare two strings
 *  this is just a wrapper for strncmp
 *  except in the Finnish build, it does Finnish
 *  localized comparison
 * TO DO - review this for localization
 *=================================================*/
int
ll_strncmp (char *str1, char *str2, int len)
{
	if(opt_finnish) return(MY_STRNCMP(str1, str2, len));
	/* TO DO - scan thru letters with custom sort order */
	return(strncmp(str1, str2, len));
}
/*===================================================
 * set_usersort -- Install custom sort
 * this will be used by llstrcmp
 *=================================================*/
void
set_usersort (usersortfnc fnc)
{
	usersort = fnc;
}

/*===================================================
 * widecmp -- Perform unicode string comparison, if availalble
 *=================================================*/
static BOOLEAN
widecmp (char *str1, char *str2, INT *rtn)
{
	bfptr bfs1=0, bfs2=0;
	BOOLEAN success = FALSE;
#ifdef HAVE_WCSCOLL
	if (int_utf8) {
#ifdef _WIN32
		/* MS-Windows really only handles UCS-2 */
		CNSTRING dest = "UCS-2-INTERNAL";
#else
		CNSTRING dest = "UCS-4-INTERNAL";
#endif
		BOOLEAN success;
		bfs1 = bfNew(strlen(str1)*4+3);
		bfCpy(bfs1, str1);
		bfs1 = iconv_trans("UTF-8", dest, bfs1, "?", &success);
		if (!success) goto failwidecmp;

		bfs2 = bfNew(strlen(str2)*4+3);
		bfCpy(bfs2, str2);
		bfs2 = iconv_trans("UTF-8", dest, bfs2, "?", &success);
		if (!success) goto failwidecmp;
		
		*rtn = wcscoll((const wchar_t *)bfs1->str, (const wchar_t *)bfs2->str);
		success = TRUE;
	}
#endif

failwidecmp:
	if (bfs1)
		bfDelete(bfs1);
	if (bfs2)
		bfDelete(bfs2);
	return success;
}
