/*+*****************************************************************************
*                                                                              *
* C dirq clock support                                                         *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2024
 */

/*
 * includes
 */

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/*
 * functions
 */

static void clock_setup (dirq_t dirq);
static void clock_cleanup (dirq_t dirq);
