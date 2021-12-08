/*+*****************************************************************************
*                                                                              *
* C dirq miscellaneous support                                                 *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2021
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
