/*+*****************************************************************************
*                                                                              *
* C dirq low level utilities                                                   *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2016
 */

/*
 * macros
 */

#define ERROR strerror(errno)
#define UNUSED(_x) do { (void)(_x); } while (0)

/*
 * functions
 */

static void die (const char *fmt, ...);
static void *safe_malloc (size_t size);
static void *safe_realloc (void *oldptr, size_t size);
