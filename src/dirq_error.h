/*+*****************************************************************************
*                                                                              *
* C dirq error support                                                         *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2024
 */

/*
 * functions
 */

static void error_set (dirq_t dirq, int errcode, const char *fmt, ...);
