/*+*****************************************************************************
*                                                                              *
* C implementation of the simple directory queue algorithm                     *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2021
 */

/*
 * includes
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include "dirq.h"
#include "dirq_clock.h"
#include "dirq_error.h"
#include "dirq_iter.h"
#include "dirq_low.h"
#include "dirq_misc.h"
#include "dirq_oo.h"

/*
 * constants
 */

#define LOCKED_SUFFIX    ".lck"
#define TEMPORARY_SUFFIX ".tmp"
#define DIR_NAME_LENGTH   8
#define ELT_NAME_LENGTH  14
#define ELEMENT_LENGTH   (DIR_NAME_LENGTH + 1 + ELT_NAME_LENGTH)

/*
 * macros
 */

#define TMP1BUF(_d)  (_d->buffer + _d->tmp1_offset)
#define TMP2BUF(_d)  (_d->buffer + _d->tmp2_offset)
#define TMP1NAME(_d) (_d->buffer + _d->tmp1_offset + _d->pathlen +  1)
#define TMP2NAME(_d) (_d->buffer + _d->tmp2_offset + _d->pathlen +  1)

/*
 * dirq_add(DIRQ, BUFFER, LENGTH): NAME success | NULL error
 */

const char *dirq_add (dirq_t dirq, dirq_iow callback)
{
  char *tmppath;
  int fd, result, offset, done;
  char buffer[8192];

  tmppath = TMP1BUF(dirq);
  /* setup the insertion directory */
  result = set_insertion_directory(dirq);
  if (result != 0)
    return(NULL);
  /* create new path to hold data */
  while (1) {
    set_new_name(dirq, dirq->tmp1_offset);
    strcpy(TMP1NAME(dirq) + ELEMENT_LENGTH, TEMPORARY_SUFFIX);
    fd = open(tmppath, O_WRONLY|O_CREAT|O_EXCL, 0666);
    if (fd >= 0)
      break;
    if (errno != EEXIST) {
      error_set(dirq, errno, "cannot open(%s): %s", tmppath, ERROR);
      return(NULL);
    }
  }
  /* save data into new path */
  while (1) {
    result = callback(dirq, buffer, sizeof(buffer));
    if (result == 0)
      break;
    if (result < 0) {
      error_set(dirq, result, "cannot write(%s): %d", tmppath, result);
      (void) close(fd); /* best effort cleanup... */
      return(NULL);
    }
    offset = 0;
    while (offset < result) {
      done = write(fd, &buffer[offset], result-offset);
      if (done < 0) {
        error_set(dirq, result, "cannot write(%s): %s", tmppath, ERROR);
        (void) close(fd); /* best effort cleanup... */
        return(NULL);
      }
      offset += done;
    }
  }
  if (close(fd) != 0) {
      error_set(dirq, errno, "cannot close(%s): %s", tmppath, ERROR);
      return(NULL);
  }
  /* add the newly created path */
  result = add_temporary_path(dirq, tmppath);
  if (result != 0)
    return(NULL);
  /* return the element name */
  return(TMP2NAME(dirq));
}

/*
 * dirq_add_path(DIRQ, PATH): NAME success | NULL error
 */

const char *dirq_add_path (dirq_t dirq, const char *path)
{
  int result;

  /* setup the insertion directory */
  result = set_insertion_directory(dirq);
  if (result != 0)
    return(NULL);
  /* directly add the path (that must be on the same filesystem) */
  result = add_temporary_path(dirq, path);
  if (result != 0)
    return(NULL);
  /* return the element name */
  return(TMP2NAME(dirq));
}

/*
 * dirq_lock(DIRQ, NAME, FLAG): 0 success | -1 error | 1 failed but permissive
 */

int dirq_lock (dirq_t dirq, const char *name, int permissive)
{
  assert(strlen(name) == ELEMENT_LENGTH);
  strcpy(TMP1NAME(dirq), name);
  strcpy(TMP2NAME(dirq), name);
  strcpy(TMP2NAME(dirq) + ELEMENT_LENGTH, LOCKED_SUFFIX);
  if (link(TMP1BUF(dirq), TMP2BUF(dirq)) != 0) {
    if (permissive && (errno == ENOENT || errno == EEXIST))
      return(1);
    error_set(dirq, errno, "cannot link(%s, %s): %s",
              TMP1BUF(dirq), TMP2BUF(dirq), ERROR);
    return(-1);
  }
  /* we also touch the element to indicate the lock time */
  if (utime(TMP1BUF(dirq), NULL) != 0) {
    if (permissive && errno == ENOENT) {
      (void) unlink(TMP2BUF(dirq)); /* best effort cleanup... */
      return(1);
    }
    error_set(dirq, errno, "cannot utime(%s, NULL): %s", TMP1BUF(dirq), ERROR);
    return(-1);
  }
  return(0);
}

/*
 * dirq_unlock(DIRQ, NAME, FLAG): 0 success | -1 error | 1 failed but permissive
 */

int dirq_unlock (dirq_t dirq, const char *name, int permissive)
{
  assert(strlen(name) == ELEMENT_LENGTH);
  strcpy(TMP2NAME(dirq), name);
  strcpy(TMP2NAME(dirq) + ELEMENT_LENGTH, LOCKED_SUFFIX);
  if (unlink(TMP2BUF(dirq)) != 0) {
    if (permissive && errno == ENOENT)
      return(1);
    error_set(dirq, errno, "cannot unlink(%s): %s", TMP2BUF(dirq), ERROR);
    return(-1);
  }
  return(0);
}

/*
 * dirq_remove(DIRQ, NAME): 0 success | -1 error
 */

int dirq_remove (dirq_t dirq, const char *name)
{
  assert(strlen(name) == ELEMENT_LENGTH);
  strcpy(TMP1NAME(dirq), name);
  strcpy(TMP2NAME(dirq), name);
  strcpy(TMP2NAME(dirq) + ELEMENT_LENGTH, LOCKED_SUFFIX);
  if (unlink(TMP1BUF(dirq)) != 0) {
    error_set(dirq, errno, "cannot unlink(%s): %s", TMP1BUF(dirq), ERROR);
    return(-1);
  }
  if (unlink(TMP2BUF(dirq)) != 0) {
    error_set(dirq, errno, "cannot unlink(%s): %s", TMP2BUF(dirq), ERROR);
    return(-1);
  }
  return(0);
}

/*
 * dirq_get(DIRQ, NAME, CALLBACK): 0 success | -1 error
 */

int dirq_get (dirq_t dirq, const char *name, dirq_ior callback)
{
  char *lckpath;
  int fd, result, done;
  char buffer[8192];

  lckpath = TMP2BUF(dirq);
  assert(strlen(name) == ELEMENT_LENGTH);
  strcpy(TMP2NAME(dirq), name);
  strcpy(TMP2NAME(dirq) + ELEMENT_LENGTH, LOCKED_SUFFIX);
  fd = open(lckpath, O_RDONLY);
  if (fd < 0) {
    error_set(dirq, errno, "cannot open(%s): %s", lckpath, ERROR);
    return(-1);
  }
  while (1) {
    done = read(fd, buffer, sizeof(buffer));
    if (done < 0) {
      error_set(dirq, result, "cannot read(%s): %s", lckpath, ERROR);
      (void) close(fd); /* best effort cleanup... */
      return(-1);
    }
    result = callback(dirq, buffer, done);
    if (result != 0) {
      error_set(dirq, result, "cannot read(%s): %d", lckpath, result);
      (void) close(fd); /* best effort cleanup... */
      return(-1);
    }
    if (done == 0)
      break;
  }
  if (close(fd) != 0) {
    error_set(dirq, errno, "cannot close(%s): %s", lckpath, ERROR);
    return(-1);
  }
  return(0);
}

/*
 * dirq_touch(DIRQ, NAME): 0 success | -1 error
 */

int dirq_touch (dirq_t dirq, const char *name)
{
  assert(strlen(name) == ELEMENT_LENGTH);
  strcpy(TMP1NAME(dirq), name);
  if (utimes(TMP1BUF(dirq), NULL) != 0) {
    error_set(dirq, errno, "cannot utimes(%s, NULL): %s", TMP1BUF(dirq), ERROR);
    return(-1);
  }
  return(0);
}

/*
 * dirq_get_size(DIRQ, NAME): >=0 success | -1 error
 */

int dirq_get_size (dirq_t dirq, const char *name)
{
  struct stat ss;

  assert(strlen(name) == ELEMENT_LENGTH);
  strcpy(TMP1NAME(dirq), name);
  if (stat(TMP1BUF(dirq), &ss) != 0) {
    error_set(dirq, errno, "cannot stat(%s): %s", TMP1BUF(dirq), ERROR);
    return(-1);
  }
  return(ss.st_size);
}

/*
 * dirq_get_path(DIRQ, NAME): PATH
 */

const char *dirq_get_path (dirq_t dirq, const char *name)
{
  if (name == NULL) {
    /* get dirq path */
    return(dirq->buffer);
  } else {
    /* get element path */
    assert(strlen(name) == ELEMENT_LENGTH);
    strcpy(TMP2NAME(dirq), name);
    strcpy(TMP2NAME(dirq) + ELEMENT_LENGTH, LOCKED_SUFFIX);
    return(TMP2BUF(dirq));
  }
}

/*
 * include code split into multiple files for maintenance purposes
 */

#include "dirq_clock.c"
#include "dirq_error.c"
#include "dirq_iter.c"
#include "dirq_low.c"
#include "dirq_misc.c"
#include "dirq_oo.c"
