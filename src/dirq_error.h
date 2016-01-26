/*+*****************************************************************************
*                                                                              *
* C dirq error support                                                         *
*                                                                              *
**-****************************************************************************/

/*
 * $Id: dirq_error.h,v 1.3 2013/04/18 15:31:07 lionel Exp $
 */

/*
 * functions
 */

static void error_set (dirq_t dirq, int errcode, const char *fmt, ...);
