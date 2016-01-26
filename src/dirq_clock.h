/*+*****************************************************************************
*                                                                              *
* C dirq clock support                                                         *
*                                                                              *
**-****************************************************************************/

/*
 * $Id: dirq_clock.h,v 1.2 2013/04/18 15:27:43 lionel Exp $
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
