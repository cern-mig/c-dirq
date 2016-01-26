/*+*****************************************************************************
*                                                                              *
* C dirq low level utilities                                                   *
*                                                                              *
**-****************************************************************************/

/*
 * $Id: dirq_low.h,v 1.2 2013/04/21 11:55:50 lionel Exp $
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
