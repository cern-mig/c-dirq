/*+*****************************************************************************
*                                                                              *
* C dirq object oriented support                                               *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2024
 */

/*
 * types
 */

struct dirq_s {
  char        *buffer;        /* allocated multi-purpose buffer */
  int          allocated;     /* size of the buffer */
  int          pathlen;       /* length of the directory queue path */
  int          tmp1_offset;   /* offset to first temporary path */
  int          tmp2_offset;   /* offset to second temporary path */
  int          dirs_offset;   /* offset to cached directories */
  int          dirs_count;    /* number of cached directories */
  int          dirs_index;    /* index of next cached directory */
  int          elts_offset;   /* offset to cached elements */
  int          elts_count;    /* number of cached elements */
  int          elts_index;    /* index of next cached element */
  int          errcode;       /* code of the "current" error */
  mode_t       umask;         /* umask to use */
  int          granularity;   /* granularity to use */
  int          rndhex;        /* random hexadecimal digit to use */
  int          maxlock;       /* maximum age for a lock before purge */
  int          maxtemp;       /* maximum age for a temp file before purge */
#ifdef __MACH__
  clock_serv_t clock;         /* Mac OS X clock */
#endif
};
