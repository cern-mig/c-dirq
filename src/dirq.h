/*+*****************************************************************************
*                                                                              *
* C implementation of the simple directory queue algorithm                     *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2016
 */

#ifndef _DIRQ_H
#define _DIRQ_H 1

/*
 * includes
 */

#include <sys/types.h>
#include <time.h>

/*
 * C++ compatibility
 */

#ifdef __cplusplus
extern "C" {
#endif

/*** BEGIN SYNOPSIS ***/

/*
 * types
 */

typedef struct dirq_s *dirq_t;
typedef int (*dirq_iow)(dirq_t, char *, size_t);
typedef int (*dirq_ior)(dirq_t, const char *, size_t);

/*
 * constructors & destructor
 */

dirq_t dirq_new  (const char *path);
dirq_t dirq_copy (dirq_t dirq);
void   dirq_free (dirq_t dirq);

/*
 * accessors
 */

void   dirq_set_granularity (dirq_t dirq, int value);
int    dirq_get_granularity (dirq_t dirq);
void   dirq_set_rndhex      (dirq_t dirq, int value);
int    dirq_get_rndhex      (dirq_t dirq);
void   dirq_set_umask       (dirq_t dirq, mode_t value);
mode_t dirq_get_umask       (dirq_t dirq);
void   dirq_set_maxlock     (dirq_t dirq, int value);
int    dirq_get_maxlock     (dirq_t dirq);
void   dirq_set_maxtemp     (dirq_t dirq, int value);
int    dirq_get_maxtemp     (dirq_t dirq);

/*
 * iterators
 */

const char *dirq_first (dirq_t dirq);
const char *dirq_next  (dirq_t dirq);

/*
 * main methods
 */

const char *dirq_add      (dirq_t dirq, dirq_iow cb);
const char *dirq_add_path (dirq_t dirq, const char *path);
int         dirq_get      (dirq_t dirq, const char *name, dirq_ior cb);
const char *dirq_get_path (dirq_t dirq, const char *name);
int         dirq_lock     (dirq_t dirq, const char *name, int permissive);
int         dirq_unlock   (dirq_t dirq, const char *name, int permissive);
int         dirq_remove   (dirq_t dirq, const char *name);
int         dirq_touch    (dirq_t dirq, const char *name);
int         dirq_get_size (dirq_t dirq, const char *name);
int         dirq_count    (dirq_t dirq);
int         dirq_purge    (dirq_t dirq);

/*
 * other methods
 */

void        dirq_now         (dirq_t dirq, struct timespec *ts);
int         dirq_get_errcode (dirq_t dirq);
const char *dirq_get_errstr  (dirq_t dirq);
void        dirq_clear_error (dirq_t dirq);

/*** END SYNOPSIS ***/

/*
 * C++ compatibility
 */

#ifdef __cplusplus
}
#endif

#endif /* _DIRQ_H */
