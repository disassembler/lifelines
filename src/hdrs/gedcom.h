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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * gedcom.h -- Main header file of LifeLines system
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 09 Dec 94
 *   3.0.3 - 17 Jan 96
 *===========================================================*/

#ifndef _GEDCOM_H
#define _GEDCOM_H

#include "table.h"
#include "translat.h"

#ifndef INCLUDED_UIPROMPTS_H
#include "uiprompts.h"
#endif

#define NAMESEP '/'
#define MAXGEDNAMELEN 512
/* keys run up to 9,999,999, eg I9999999 */
/* These are GEDCOM identifier numbers, eg, for @I510@,
 the key is 510 */
/* btree databases use this as part of the database key */
#define MAXKEYWIDTH 8
#define MAXKEYNUMBER 9999999

#define OKAY  1
#define ERROR 0
#define DONE -1

/*=====================================
 * NODE -- Internal form of GEDCOM line
 *===================================*/
/*
 A NODE is a struct ntag which holds the in-memory
 representation of one line of a GEDCOM record.
 For example, a NODE may hold the in-memory representation
 of the GEDCOM line "2 DATE ABT 1900". The NODE actually contains
 the "DATE" part of the string in its n_tag field, and
 the "ABT 1900" portion of the string in its n_val field.
 The level is not explicitly given, but is implicit in the
 NODEs location in the NODE tree in which it lives (it has
 fields n_parent, n_child, n_sibling which connect it into
 its NODE tree). (E.g., its parent might be a NODE representing
 "1 BIRT".)
*/
typedef struct tag_cacheel *CACHEEL;
typedef struct tag_node *NODE;
struct tag_node {
	STRING n_xref;      /* cross ref */
	STRING n_tag;       /* tag */
	STRING n_val;       /* value */
	NODE   n_parent;    /* parent */
	NODE   n_child;     /* first child */
	NODE   n_sibling;   /* sibling */
	INT    n_flag;      /* eg, ND_TEMP */
	int    n_refcnt;    /* refcount for temp nodes */
	CACHEEL n_cel;      /* pointer to cacheel, if node is inside cache */
};
#define nxref(n)    ((n)->n_xref)
#define ntag(n)     ((n)->n_tag)
#define nval(n)     ((n)->n_val)
#define nparent(n)  ((n)->n_parent)
#define nchild(n)   ((n)->n_child)
#define nsibling(n) ((n)->n_sibling)
#define nflag(n)    ((n)->n_flag)
#define nrefcnt(n)  ((n)->n_refcnt)
enum { ND_TEMP=1 };

struct tag_nkey { char ntype; INT keynum; char key[MAXKEYWIDTH+1]; };
typedef struct tag_nkey NKEY;

/*=====================================
 * RECORD -- Internal form of GEDCOM record
 *===================================*/
/*
 A RECORD is a struct record_s which holds the in-memory representation
 of an entire GEDCOM record, such as an INDI. It has a pointer to the
 root NODE of the INDI (which is of course a NODE representing a line
 such as "0 @I43@ INDI"), and it also contains some additional data.
 A RECORD will in the future contain a pointer to its cache element.
 LifeLines is very RECORD-oriented.
*/

typedef struct tag_record *RECORD;
struct tag_record { /* RECORD */
	NODE rec_top;           /* for non-cache records */
	NKEY rec_nkey;
	struct tag_cacheel * rec_cel; /* cache wrapper */
};
NODE nztop(RECORD); /* handles NULL, also reloads from cache */
#define nzkey(n)    ((n)->rec_nkey.key)
#define nzkeynum(n) ((n)->rec_nkey.keynum)
#define nztype(n)   ((n)->rec_nkey.ntype)

/*=====================================
 * LLDATABASE types -- LifeLines database
 *===================================*/

typedef struct tag_lldatabase *LLDATABASE;

/*
 reformating functions - format date or place for display
 callbacks to GUI client to tailor display as desired
 either or both may be null, meaning use date or place exactly as
 occurs in data
*/
struct tag_rfmt {
	STRING (*rfmt_date)(STRING); /* returns static buffer */
	STRING (*rfmt_plac)(STRING); /* returns static buffer */
	STRING combopic; /* stdalloc'd buffer, eg, "%1, %2" */
};
typedef struct tag_rfmt *RFMT;


/*==============================================
 * Option type enumerations (but we use defines)
 *============================================*/

#define SEX_MALE    1
#define SEX_FEMALE  2
#define SEX_UNKNOWN 3


/* custom translation tables */
	/* MEDIN: translate editor characters to internal */
#define MEDIN 0
	/* MINED: translate internal characters to editor */
#define MINED 1
	/* MGDIN: translate gedcom file characters to internal */
#define MGDIN 2
	/* MGDIN: translate internal characters to gedcom file */
#define MINGD 3
	/* MDSIN: translate display characters to internal */
#define MDSIN 4
	/* MINDS: translate internal characters to display */
#define MINDS 5
	/* MRPIN: translate report to internal characters */
#define MRPIN 6
	/* MINRP: translate internal characters to report */
#define MINRP 7
	/* MSORT: custom sort table, characters to numeric order */
#define MSORT 8
	/* MCHAR: character table (translation result unused) */
#define MCHAR 9
	/* MLCAS: translate character to lower-case (UNIMPLEMENTED) */
#define MLCAS 10
	/* MUCAS: translate character to upper-case (UNIMPLEMENTED) */
#define MUCAS 11
	/* MPREF: prefix to skip for sorting (UNIMPLEMENTED) */
#define MPREF 12
	/* number of maps listed above */
#define NUM_TT_MAPS 13

/*========
 * Globals
 *======*/

extern INT flineno;
extern BOOLEAN keyflag;
extern BOOLEAN readonly;
extern BOOLEAN immutable;
extern STRING editstr;
extern STRING editfile;
/* tabtable & placabbvs should be moved into LLDATABASE */
extern TABLE tagtable;		/* table for GEDCOM tags */
extern TABLE placabbvs;		/* table for place abbrvs */
extern LLDATABASE def_lldb;        /* default database */


/*=====================
 * enums for functions
 *===================*/
typedef enum { DOSURCAP, NOSURCAP } SURCAPTYPE;
typedef enum { SURFIRST, REGORDER } SURORDER;

/*=====================
 * Function definitions
 *===================*/

STRING addat(STRING);
void addixref(INT key);
void addexref(INT key);
void addfxref(INT key);
void addsxref(INT key);
void addxref(STRING key);
void addxxref(INT key);
BOOLEAN add_refn(STRING, STRING);
BOOLEAN are_locales_supported(void);
RECORD choose_child(RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1);
void choose_and_remove_family(void);
RECORD choose_father(RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1);
RECORD choose_family(RECORD irec, STRING msg0, STRING msgn, BOOLEAN fams);
RECORD choose_mother(RECORD indi, RECORD fam, STRING msg0, STRING msgn, ASK1Q ask1);
RECORD choose_note(RECORD current, STRING msg0, STRING msgn);
RECORD choose_pointer(RECORD current, STRING msg0, STRING msgn);
RECORD choose_source(RECORD current, STRING msg0, STRING msgn);
RECORD choose_spouse(RECORD irec, STRING msg0, STRING msgn);
void classify_nodes(NODE*, NODE*, NODE*);
void closexref(void);
void close_lifelines(void);
NODE convert_first_fp_to_node(FILE*, BOOLEAN, XLAT, STRING*, BOOLEAN*);
NODE copy_node(NODE);
NODE copy_nodes(NODE, BOOLEAN, BOOLEAN);
BOOLEAN create_database(STRING dbused);
NODE create_node(STRING, STRING, STRING, NODE);
NODE create_temp_node(STRING, STRING, STRING, NODE);
void del_in_dbase(STRING key);
void delete_metarec(STRING key);
BOOLEAN edit_mapping(INT);
BOOLEAN edit_valtab_from_db(STRING, TABLE*, INT sep, STRING, STRING (*validator)(TABLE tab));
BOOLEAN equal_tree(NODE, NODE);
BOOLEAN equal_node(NODE, NODE);
BOOLEAN equal_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void even_to_cache(NODE);
void even_to_dbase(NODE);
STRING even_to_list_string(NODE even, INT len, STRING delim);
STRING event_to_date(NODE, BOOLEAN);
void event_to_date_place(NODE node, STRING * date, STRING * plac);
STRING event_to_plac(NODE, BOOLEAN);
STRING event_to_string(NODE, RFMT rfmt);
void fam_to_cache(NODE);
void fam_to_dbase(NODE);
STRING fam_to_event(NODE, STRING tag, STRING head, INT len, RFMT);
NODE fam_to_first_chil(NODE);
NODE fam_to_last_chil(NODE);
STRING fam_to_list_string(NODE fam, INT len, STRING delim);
NODE fam_to_spouse(NODE, NODE);
int next_spouse(NODE *node, RECORD *spouse);
RECORD file_to_record(STRING fname, XLAT ttm, STRING *pmsg, BOOLEAN *pemp);
NODE file_to_node(STRING, XLAT, STRING*, BOOLEAN*);
INT file_to_line(FILE*, XLAT, INT*, STRING*, STRING*, STRING*, STRING*);
NODE find_node(NODE, STRING, STRING, NODE*);
NODE find_tag(NODE, CNSTRING);
void free_cached_rec(RECORD rec);
void free_rec(RECORD);
void free_node(NODE);
void free_nodes(NODE);
void free_temp_node_tree(NODE);
STRING full_value(NODE, STRING sep);
ZSTR get_cache_stats_fam(void);
ZSTR get_cache_stats_indi(void);
STRING get_current_locale_collate(void);
STRING get_current_locale_msgs(void);
INT get_dblist(STRING path, LIST * dblist, LIST * dbdesclist);
INT get_decimal(STRING);
INT get_hexidecimal(STRING);
STRING get_lifelines_version(INT maxlen);
STRING get_original_locale_collate(void);
STRING get_original_locale_msgs(void);
STRING get_property(STRING opt);
void get_refns(STRING, INT*, STRING**, INT);
STRING getexref(void);
STRING getfxref(void);
INT getixrefnum(void);
STRING getsxref(void);
STRING getxxref(void);
BOOLEAN getrefnrec(CNSTRING refn);
void growexrefs(void);
void growfxrefs(void);
void growixrefs(void);
void growsxrefs(void);
void growxxrefs(void);
INT hexvalue(INT);
BOOLEAN in_string(INT, STRING);
void index_by_refn(NODE, STRING);
void indi_to_dbase(NODE);
STRING indi_to_event(NODE, STRING tag, STRING head, INT len, RFMT);
NODE indi_to_famc(NODE);
NODE indi_to_fath(NODE);
NODE indi_to_moth(NODE);
STRING indi_to_name(NODE, INT);
RECORD indi_to_next_sib(RECORD);
NODE indi_to_next_sib_old(NODE);
STRING indi_to_ped_fix(NODE indi, INT len);
RECORD indi_to_prev_sib(RECORD);
NODE indi_to_prev_sib_old(NODE);
STRING indi_to_title(NODE, INT);
void initxref(void);
void init_browse_lists(void);
void init_caches(void);
void free_caches(void);
void init_disp_reformat(void);
BOOLEAN init_lifelines_db(void);
BOOLEAN init_lifelines_global(STRING configfile, STRING * pmsg, void (*notify)(STRING db, BOOLEAN opening));
CNSTRING init_get_config_file(void);
void init_new_record(RECORD rec, char ntype, INT keynum);
BOOLEAN init_valtab_from_file(STRING, TABLE, XLAT, INT sep, STRING*);
BOOLEAN init_valtab_from_rec(CNSTRING, TABLE, INT sep, STRING*);
BOOLEAN init_valtab_from_string(CNSTRING, TABLE, INT sep, STRING*);
BOOLEAN is_codeset_utf8(CNSTRING codeset);
BOOLEAN is_iconv_supported(void);
BOOLEAN is_nls_supported(void);
BOOLEAN is_temp_node(NODE);
BOOLEAN iso_list(NODE, NODE);
BOOLEAN iso_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void join_fam(NODE, NODE, NODE, NODE, NODE, NODE);
void join_indi(NODE, NODE, NODE, NODE, NODE, NODE, NODE);
void join_othr(NODE root, NODE refn, NODE rest);
STRING key_of_record(NODE);
RECORD key_possible_to_record(STRING, INT let);
NODE key_to_even(CNSTRING);
RECORD key_to_erecord(CNSTRING);
NODE key_to_fam(CNSTRING);
RECORD key_to_frecord(CNSTRING);
NODE key_to_indi(CNSTRING);
RECORD key_to_irecord(CNSTRING);
RECORD key_to_orecord(CNSTRING);
NODE key_to_othr(CNSTRING);
RECORD key_to_record(CNSTRING key);
NODE key_to_sour(CNSTRING);
RECORD key_to_srecord(CNSTRING);
NODE key_to_type(CNSTRING key, INT reportmode);
NODE keynum_to_fam(int keynum);
RECORD keynum_to_erecord(int keynum);
RECORD keynum_to_frecord(int keynum);
NODE keynum_to_indi(int keynum);
RECORD keynum_to_irecord(int keynum);
NODE keynum_to_node(char ntype, int keynum);
RECORD keynum_to_orecord(int keynum);
RECORD keynum_to_record(char ntype, int keynum);
RECORD keynum_to_srecord(int keynum);
NODE keynum_to_sour(int keynum);
NODE keynum_to_even(int keynum);
NODE keynum_to_othr(int keynum);
INT length_nodes(NODE);
STRING ll_langinfo(void);
char * llsetlocale(int category, char * locale);
void load_char_mappings(void);
BOOLEAN load_new_tt(CNSTRING filepath, INT ttnum);
void locales_notify_language_change(void);
void locales_notify_uicodeset_changes(void);
STRING newexref(STRING, BOOLEAN);
STRING newfxref(STRING, BOOLEAN);
STRING newixref(STRING, BOOLEAN);
STRING newsxref(STRING, BOOLEAN);
STRING newxxref(STRING, BOOLEAN);
void new_name_browse_list(STRING, STRING);
RECORD next_fp_to_record(FILE*, BOOLEAN, XLAT, STRING*, BOOLEAN*);
NODE next_fp_to_node(FILE*, BOOLEAN, XLAT, STRING*, BOOLEAN*);
void nkey_copy(NKEY * src, NKEY * dest);
BOOLEAN nkey_eq(NKEY * nkey1, NKEY * nkey2);
void nkey_clear(NKEY * nkey);
void nkey_load_key(NKEY * nkey);
BOOLEAN nkey_to_node(NKEY * nkey, NODE * node);
BOOLEAN nkey_to_record(NKEY * nkey, RECORD * prec);
NKEY nkey_zero(void);
void node_to_dbase(NODE, STRING);
BOOLEAN node_to_file(INT, NODE, STRING, BOOLEAN, TRANTABLE);
INT node_to_keynum(char ntype, NODE nod);
NODE node_to_next_event(NODE node, CNSTRING tag);
BOOLEAN node_to_nkey(NODE node, NKEY * nkey);
NODE node_to_node(NODE, INT*);
RECORD node_to_record(NODE node);
STRING node_to_tag(NODE node, STRING tag, INT len);
INT num_evens(void);
INT num_fam_xrefs(NODE fam);
INT num_fams(void);
INT num_indis(void);
INT num_spouses_of_indi(NODE);
INT num_sours(void);
INT num_othrs(void);
BOOLEAN open_database(INT alteration, STRING dbused);
BOOLEAN openxref(BOOLEAN readonly);
STRING other_to_list_string(NODE node, INT len, STRING delim);
void othr_to_cache(NODE);
void othr_to_dbase(NODE);
BOOLEAN place_to_list(STRING, LIST, INT*);
BOOLEAN pointer_value(STRING);
NODE qkey_to_even(CNSTRING key);
RECORD qkey_to_erecord(CNSTRING key);
NODE qkey_to_fam(CNSTRING key);
RECORD qkey_to_frecord(CNSTRING key);
NODE qkey_to_indi(CNSTRING key);
RECORD qkey_to_irecord(CNSTRING key);
RECORD qkey_to_orecord(CNSTRING key);
NODE qkey_to_othr(CNSTRING key);
RECORD qkey_to_record(CNSTRING key);
NODE qkey_to_sour(CNSTRING key);
RECORD qkey_to_srecord(CNSTRING key);
NODE qkey_to_type(CNSTRING key);
RECORD qkeynum_to_frecord(int keynum);
NODE qkeynum_to_indi(int keynum);
INT record_letter(CNSTRING);
void record_to_date_place(RECORD record, STRING tag, STRING * date, STRING * plac);
NODE record_to_first_event(RECORD record, CNSTRING tag);
NODE refn_to_record(STRING, INT);
void register_uicodeset_callback(CALLBACK_FNC fncptr, VPTR uparm);
void register_uilang_callback(CALLBACK_FNC fncptr, VPTR uparm);
void release_dblist(LIST dblist);
BOOLEAN remove_child(NODE indi, NODE fam);
BOOLEAN remove_empty_fam(NODE);
BOOLEAN remove_any_record(RECORD record);
void remove_indi_by_root(NODE);
void remove_indi_cache(STRING key);
void remove_fam_cache(STRING key);
void remove_from_cache_by_key(STRING key);
BOOLEAN remove_fam_record(RECORD frec);
void remove_from_browse_lists(STRING);
BOOLEAN remove_refn(CNSTRING refn, CNSTRING key);
void rename_from_browse_lists(STRING);
BOOLEAN remove_spouse(NODE indi, NODE fam);
void replace_fam(NODE fam1, NODE fam2);
void replace_indi(NODE indi1, NODE indi2);
INT resolve_refn_links(NODE);
RECORD_STATUS retrieve_to_file(STRING key, STRING file);
RECORD_STATUS retrieve_to_textfile(STRING key, STRING file, TRANSLFNC);
STRING retrieve_raw_record(CNSTRING, INT*);
STRING rmvat(STRING);
STRING rmvbrackets(STRING str);
STRING rpt_setlocale(STRING str);
void rptlocale(void);
void save_original_locales(void);
BOOLEAN save_tt_to_file(INT ttnum, STRING filename);
void set_displaykeys(BOOLEAN);
void set_gettext_codeset(CNSTRING domain, CNSTRING codeset);
void set_temp_node(NODE, BOOLEAN temp);
STRING shorten_plac(STRING);
void show_node(NODE node);
void show_node_rec(INT, NODE);
void sour_to_cache(NODE);
void sour_to_dbase(NODE);
STRING sour_to_list_string(NODE sour, INT len, STRING delim);
void split_fam(NODE, NODE*, NODE*, NODE*, NODE*, NODE*);
void split_indi_old(NODE, NODE*, NODE*, NODE*, NODE*, NODE*, NODE*);
void split_othr(NODE node, NODE *prefn, NODE *prest);
BOOLEAN store_file_to_db(STRING key, STRING file);
BOOLEAN store_record(STRING key, STRING rec, INT len);
BOOLEAN store_text_file_to_db(STRING key, CNSTRING file, TRANSLFNC);
RECORD string_to_record(STRING str, CNSTRING key, INT len);
void termlocale(void);
void traverse_db_key_recs(BOOLEAN(*func)(CNSTRING key, RECORD, void *param), void *param);
void traverse_db_rec_keys(CNSTRING lo, CNSTRING hi, BOOLEAN(*func)(CNSTRING key, STRING data, INT, void *param), void * param);
BOOLEAN traverse_nodes(NODE node, BOOLEAN (*func)(NODE, VPTR), VPTR param);
void traverse_refns(BOOLEAN(*func)(CNSTRING key, CNSTRING refn, BOOLEAN newset, void *param), void *param);
INT tree_strlen(INT, NODE);
void uilocale(void);
NODE union_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
NODE unique_nodes(NODE, BOOLEAN);
void unknown_node_to_dbase(NODE node);
void unregister_uilang_callback(CALLBACK_FNC fncptr, VPTR uparm);
void unregister_uicodeset_callback(CALLBACK_FNC fncptr, VPTR uparm);
void update_useropts(VPTR uparm);
BOOLEAN valid_indi_tree(NODE, STRING*, NODE);
BOOLEAN valid_fam_tree(NODE, STRING*, NODE);
BOOLEAN valid_name(STRING);
BOOLEAN valid_node_type(NODE node, char ntype, STRING *pmsg, NODE node0);
BOOLEAN valid_sour_tree(NODE, STRING*, NODE);
BOOLEAN valid_to_list(STRING, LIST, INT*, STRING);
BOOLEAN valid_even_tree(NODE, STRING*, NODE);
BOOLEAN valid_othr_tree(NODE, STRING*, NODE);
INT val_to_sex(NODE);
BOOLEAN value_to_list(STRING, LIST, INT*, STRING);
STRING value_to_xref(STRING);
BOOLEAN writexrefs(void);
void write_node_to_editfile(NODE); /* used by Ethel */
INT xref_firste(void);
INT xref_firstf(void);
INT xref_firsti(void);
INT xref_firsts(void);
INT xref_firstx(void);
INT xref_laste(void);
INT xref_lastf(void);
INT xref_lasti(void);
INT xref_lasts(void);
INT xref_lastx(void);
INT xref_max_any(void);
INT xref_max_indis(void);
INT xref_max_evens (void);
INT xref_max_fams (void);
INT xref_max_othrs (void);
INT xref_max_sours (void);
INT xref_next(char ntype, INT i);
INT xref_nexte(INT);
INT xref_nextf(INT);
INT xref_nexti(INT);
INT xref_nexts(INT);
INT xref_nextx(INT);
INT xref_prev(char ntype, INT i);
INT xref_preve(INT);
INT xref_prevf(INT);
INT xref_previ(INT);
INT xref_prevs(INT);
INT xref_prevx(INT);
INT xrefval(char ntype, STRING str);

/* keytonod.c */
void add_new_indi_to_cache(RECORD rec);
RECORD create_record_for_cel(CACHEEL cel);

/* gstrings.c */
STRING generic_to_list_string(NODE node, STRING key, INT len, STRING delim, RFMT rfmt, BOOLEAN appkey);
STRING *get_child_strings(NODE, RFMT, INT*, STRING**);
STRING indi_to_list_string(NODE indi, NODE fam, INT len, RFMT rfmt, BOOLEAN appkey);

/* lldatabase.c */
void lldb_adderror(LLDATABASE lldb, int errnum, CNSTRING errstr);
LLDATABASE lldb_alloc(void);
void lldb_close(LLDATABASE lldb);
void lldb_set_btree(LLDATABASE lldb, void * btree);

/* names.c */
void add_name(STRING, STRING);
void free_name_list(LIST list);
void free_string_list(LIST list);
LIST find_indis_by_name(CNSTRING name);
CNSTRING getasurname(CNSTRING);
CNSTRING getsxsurname(CNSTRING);
CNSTRING givens(CNSTRING);
STRING manip_name(STRING name, SURCAPTYPE captype, SURORDER surorder, INT len);
BOOLEAN name_to_list(CNSTRING, LIST, INT*, INT*);
STRING name_string(STRING);
int namecmp(STRING, STRING);
void remove_name(STRING name, CNSTRING key);
void traverse_names(BOOLEAN(*func)(STRING key, CNSTRING name, BOOLEAN newset, void *param), void *param);
STRING trim_name(STRING, INT);

/* node.c */
/* node tree iterator */
struct tag_node_iter {
	NODE start;
	NODE next;
};
typedef struct tag_node_iter * NODE_ITER;
RECORD alloc_new_record(void);
void begin_node_it(NODE node, NODE_ITER nodeit);
INT fam_to_husb(RECORD frec, RECORD * prec);
NODE fam_to_husb_node(NODE);
INT fam_to_wife(RECORD frec, RECORD * prec);
NODE fam_to_wife_node(NODE);
NODE next_node_it_ptr(NODE_ITER nodeit);

/* nodeio.c */
STRING node_to_string(NODE);
NODE string_to_node(STRING);
void write_indi_to_file_for_edit(NODE indi, CNSTRING file, RFMT rfmt);
void write_fam_to_file(NODE fam, CNSTRING file);
void write_nodes(INT, FILE*, XLAT, NODE, BOOLEAN, BOOLEAN, BOOLEAN);

/* refns.c */
void annotate_with_supplemental(NODE node, RFMT rfmt);

/* soundex.c */
CNSTRING trad_soundex(CNSTRING);
INT soundex_count(void);
CNSTRING soundex_get(INT i, CNSTRING name);

/*******************
 various Macros
 *******************/

#define NAME(indi)  find_tag(nchild(indi),"NAME")
#define REFN(indi)  find_tag(nchild(indi),"REFN")
#define SEX(indi)   val_to_sex(find_tag(nchild(indi),"SEX"))
#define BIRT(indi)  find_tag(nchild(indi),"BIRT")
#define DEAT(indi)  find_tag(nchild(indi),"DEAT")
#define BAPT(indi)  find_tag(nchild(indi),"CHR")
#define BURI(indi)  find_tag(nchild(indi),"BURI")
#define FAMC(indi)  find_tag(nchild(indi),"FAMC")
#define FAMS(indi)  find_tag(nchild(indi),"FAMS")

#define HUSB(fam)   find_tag(nchild(fam),"HUSB")
#define WIFE(fam)   find_tag(nchild(fam),"WIFE")
#define MARR(fam)   find_tag(nchild(fam),"MARR")
#define CHIL(fam)   find_tag(nchild(fam),"CHIL")

#define DATE(evnt)   find_tag(nchild(evnt),"DATE")
#define PLAC(evnt)   find_tag(nchild(evnt),"PLAC")

/*=============================================
 * indi_to_key, fam_to_key - return key of node
 *  eg, "I21"
 *  returns static buffer
 *===========================================*/
#define indi_to_key(indi)  (rmvat(nxref(indi)))
#define fam_to_key(fam)    (rmvat(nxref(fam)))
#define node_to_key(node)  (rmvat(nxref(node)))

/*=============================================
 * indi_to_keynum, fam_to_keynum, etc - 
 *  return keynum of node, eg, 21
 *===========================================*/
#define indi_to_keynum(indi) (node_to_keynum('I', indi))
#define fam_to_keynum(fam)  (node_to_keynum('F', fam))
#define sour_to_keynum(sour) (node_to_keynum('S', sour))

/*=============================================
 * num_families - count spouses of indi
 *===========================================*/
#define num_families(indi) (length_nodes(FAMS(indi)))

 /*=============================================
 * num_children - count children of indi
 *===========================================*/
#define num_children(fam)  (length_nodes(CHIL(fam)))

/*
  Possibly all of these should lock their main argument in the cache
  Instead of having some of the client code do it, and some not
  Now that NODE has a pointer to its parent RECORD,
  and RECORD has a pointer to the cacheel, it is possible 
*/
/*
 FORCHILDRENx takes a node as its first arg
 FORCHILDREN takes a record as its first arg
*/

#define FORCHILDRENx(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	NODE child=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(child = key_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		++num;\
		{

#define ENDCHILDRENx \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "CHIL")) __node = NULL;\
	}}

#define FORCHILDREN(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	RECORD child=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(child = key_to_irecord(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		++num;\
		{

#define ENDCHILDREN \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "CHIL")) __node = NULL;\
	}}

/* FORSPOUSES iterate over all FAMS nodes & all spouses of indi
 * if there are multiple spouses, user will see same fam with multiple spouses
 * if there are no spouses for a particular family the family is not returned.
 */
#define FORSPOUSES(indi,spouse,fam,num) \
	{\
	NODE __node = FAMS(indi);\
	NODE __node1=0, spouse=0,fam=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
	    __key = rmvat(nval(__node));\
	    __node = nsibling(__node);\
	    if (__key && (fam = qkey_to_fam(__key))) {\
		__node1 = nchild(fam);\
		/* inline find_tag here to search for either husb or wife */\
		while (__node1) {\
		    spouse = 0;\
		    if (eqstr("HUSB",ntag(__node1)) || eqstr("WIFE",ntag(__node1))){\
			__key = rmvat(nval(__node1));\
			__node1 = nsibling(__node1);\
			if (!__key || !(spouse = key_to_indi(__key))||spouse==indi){\
			    continue;\
			}\
		    } else {\
			__node1 = nsibling(__node1);\
			continue;\
		    }\
		    ++num;\
		    {

#define ENDSPOUSES \
		    }\
		}\
	    }\
	    if (__node && nestr(ntag(__node), "FAMS")) __node = NULL;\
	}}

/* FORFAMS iterate over all FAMS nodes of indi
 *    this is an optimization of FORFAMSS for cases where spouse is not used
 *    or computed in other ways.
 */
#define FORFAMS(indi,fam,num) \
	{\
	NODE __node = FAMS(indi);\
	NODE fam=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
	    if (__key && (fam = qkey_to_fam(__key))) {\
		++num;\
		{

#define ENDFAMS \
		}\
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "FAMS")) __node = NULL;\
	}}

/* FORFAMSS iterate over all FAMS nodes & all spouses of indi
 * if there are multiple spouses, user will see same fam with multiple spouses
 * if there are no spouses for a particular family NULL is returned for spouse
 */
#define FORFAMSS(indi,fam,spouse,num) \
	{\
	INT first_sp; \
	NODE __node = FAMS(indi);\
	NODE __node1=0, spouse=0, fam=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
	    __key = rmvat(nval(__node));\
	    __node = nsibling(__node);\
	    if (__key && (fam = qkey_to_fam(__key))) {\
		__node1 = nchild(fam);\
		first_sp = 0;\
		while (__node1) {\
		    spouse=0;\
		    if (eqstr("HUSB",ntag(__node1)) || eqstr("WIFE",ntag(__node1))){\
			__key = rmvat(nval(__node1));\
			__node1 = nsibling(__node1);\
			if (!__key || !(spouse = key_to_indi(__key))||spouse==indi){\
			    continue;\
			}\
			first_sp = 1;\
		    } else {\
			__node1 = nsibling(__node1);\
			if (__node1 || first_sp) continue;\
		    }\
		++num;\
		{

#define ENDFAMSS \
		}\
	    }\
	}\
	if (__node && nestr(ntag(__node), "FAMS")) __node = NULL;\
	}}

/* FORFAMCS iterate over all parent families of indi
 * Up to one father and mother are given for each
 * (This ignores non-traditional parents)
 */
#define FORFAMCS(indi,fam,fath,moth,num) \
	{\
	NODE __node = FAMC(indi);\
	NODE fam, fath, moth;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(fam = qkey_to_fam(__key))) {\
			 ++num;\
			 __node = nsibling(__node);\
			 continue;\
		}\
		fath = fam_to_husb_node(fam);\
		moth = fam_to_wife_node(fam);\
		++num;\
		{

#define ENDFAMCS \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "FAMC")) __node = NULL;\
	}}

/* FORHUSBS iterate over all husbands in one family
 * (This handles more than one husband in a family)
 */
#define FORHUSBS(fam,husb,num) \
	{\
	NODE __node = find_tag(nchild(fam), "HUSB");\
	NODE husb=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(husb = key_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		++num;\
		{

#define ENDHUSBS \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "HUSB")) __node = NULL;\
	}}

/* FORWIFES iterate over all wives in one family
 * (This handles more than one wife in a family)
 */
#define FORWIFES(fam,wife,num) \
	{\
	NODE __node = find_tag(nchild(fam), "WIFE");\
	NODE wife=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(wife = key_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		ASSERT(wife);\
		num++;\
		{

#define ENDWIFES \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "WIFE")) __node = NULL;\
	}}

/* FORFAMSPOUSES iterate over all spouses in one family
 * (All husbands and wives)
 */
#define FORFAMSPOUSES(fam,spouse,num) \
	{\
	NODE __node = nchild(fam);\
	NODE spouse=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "HUSB") && !eqstr(ntag(__node), "WIFE")) {\
			__node = nsibling(__node);\
			continue;\
		}\
		__key = rmvat(nval(__node));\
		if (!__key || !(spouse = key_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		++num;\
		{

#define ENDFAMSPOUSES \
		}\
		__node = nsibling(__node);\
	}}

#define FORTAGVALUES(root,tag,node,value)\
	{\
	NODE node, __node = nchild(root);\
	STRING value, __value;\
	while (__node) {\
		while (__node && nestr(tag, ntag(__node)))\
			__node = nsibling(__node);\
		if (__node == NULL) break;\
		__value = value = full_value(__node, "\n");/*OBLIGATION*/\
		node = __node;\
		{
#define ENDTAGVALUES \
		}\
		if (__value) stdfree(__value);/*RELEASE*/\
		 __node = nsibling(__node);\
	}}

#endif /* _GEDCOM_H */

