/*+*****************************************************************************
*                                                                              *
* C dirq miscellaneous support                                                 *
*                                                                              *
**-****************************************************************************/

/*
 * $Id: dirq_misc.h,v 1.3 2013/04/18 16:42:04 lionel Exp $
 */

/*
 * functions
 */

static void allocate_more (dirq_t dirq);
static int ensure_directory (dirq_t dirq, const char *path);
static int ensure_directory_recursively (dirq_t dirq, const char *path);
static void set_new_name (dirq_t dirq, int offset);
static int set_insertion_directory (dirq_t dirq);
static int add_temporary_path (dirq_t dirq, const char *path);
