/*+*****************************************************************************
*                                                                              *
* C dirq clock support                                                         *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2017
 */

/*
 * clock setup
 */

static void clock_setup (dirq_t dirq)
{
#ifdef __MACH__
  kern_return_t kr;

  kr = host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &dirq->clock);
  if (kr != KERN_SUCCESS)
    die("cannot host_get_clock_service(HOST, CALENDAR_CLOCK, &clock): %s",
        mach_error_string(kr));
#else
  UNUSED(dirq);
#endif
}

/*
 * clock cleanup
 */

static void clock_cleanup (dirq_t dirq)
{
#ifdef __MACH__
  kern_return_t kr;

  kr = mach_port_deallocate(mach_task_self(), dirq->clock);
  if (kr != KERN_SUCCESS)
    die("cannot mach_port_deallocate(mach_task_self(), clock): %s",
        mach_error_string(kr));
#else
  UNUSED(dirq);
#endif
}

/*
 * return the current time in a timespec struct (any error is fatal)
 *
 * http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
 */

#ifdef __MACH__

void dirq_now (dirq_t dirq, struct timespec *ts)
{
  mach_timespec_t mts;
  kern_return_t kr;

  kr = clock_get_time(dirq->clock, &mts);
  if (kr != KERN_SUCCESS)
    die("cannot clock_get_time(CALENDAR_CLOCK, &mts): %s",
        mach_error_string(kr));
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
}

#else /* __MACH__ */

void dirq_now (dirq_t dirq, struct timespec *ts)
{
  UNUSED(dirq);
  if (clock_gettime(CLOCK_REALTIME, ts) < 0)
    die("cannot clock_gettime(CLOCK_REALTIME, &ts): %s", ERROR);
}

#endif /* __MACH__ */
