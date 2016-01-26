/*+*****************************************************************************
*                                                                              *
* C dirq object oriented support                                               *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2016
 */

/*
 * dirq_new(PATH): new object (that may be in error state, to be checked!)
 */

dirq_t dirq_new (const char *path)
{
  dirq_t dirq;
  int offset;
  struct timespec ts;

  if (strlen(path) > 2048)
    die("path too long: %s", path);
  dirq = (dirq_t)safe_malloc(sizeof(struct dirq_s));
  clock_setup(dirq);
  dirq_now(dirq, &ts);
  dirq->allocated = 8192;
  dirq->buffer = (char *)safe_malloc(dirq->allocated);
  /* set path */
  strcpy(dirq->buffer, path);
  dirq->pathlen = strlen(path);
  while (dirq->pathlen > 1 && dirq->buffer[dirq->pathlen - 1] == '/') {
    dirq->pathlen--;
    dirq->buffer[dirq->pathlen] = '\0';
  }
  /* set tmp1 */
  offset = dirq->pathlen + 1 /* NULL */ + 3 /* align */;
  offset -= offset % 4;
  dirq->tmp1_offset = offset;
  strcpy(TMP1BUF(dirq), dirq->buffer);
  dirq->buffer[dirq->tmp1_offset + dirq->pathlen] = '/';
  /* set tmp2 */
  offset = dirq->pathlen + 1 /* slash */ + 8 /* dir */ + 1 /* slash */ +
    14 /* elt */ + 4 /* suffix */ + 1 /* NULL */ + 3 /* align */;
  offset -= offset % 4;
  dirq->tmp2_offset = dirq->tmp1_offset + offset;
  strcpy(TMP2BUF(dirq), dirq->buffer);
  dirq->buffer[dirq->tmp2_offset + dirq->pathlen] = '/';
  /* reset iterator */
  dirq->dirs_offset = dirq->tmp2_offset + offset;
  dirq->elts_offset = 0;
  iter_reset(dirq);
  /* set defaults */
  dirq->granularity = 60;
  dirq->rndhex = ts.tv_nsec % 16;
  dirq->umask = 0;
  dirq->maxlock = 600;
  dirq->maxtemp = 300;
  dirq->errcode = 0;
  /* make sure toplevel directory exists (up to caller to check for success!) */
  /* this is dirty but the only way to pass back the error message... */
  (void) ensure_directory_recursively(dirq, dirq->buffer);
  return(dirq);
}

/*
 * dirq_copy(DIRQ): exact copy of the given object (rndhex, state...)
 */

dirq_t dirq_copy (dirq_t dirq1)
{
  dirq_t dirq2;

  dirq2 = (dirq_t)safe_malloc(sizeof(struct dirq_s));
  memcpy((void *)dirq2, (const void *)dirq1, sizeof(struct dirq_s));
  clock_setup(dirq2);
  dirq2->buffer = (char *)safe_malloc(dirq2->allocated);
  memcpy((void *)dirq2->buffer, (const void *)dirq1->buffer, dirq2->allocated);
  return(dirq2);
}

/*
 * dirq_free(DIRQ)
 */

void dirq_free (dirq_t dirq)
{
  clock_cleanup(dirq);
  free((void *)dirq->buffer);
  free((void *)dirq);
}

/*
 * granularity (assumed to be zero if negative)
 */

void dirq_set_granularity (dirq_t dirq, int value)
{
  dirq->granularity = (value < 0) ? 0 : value;
}

int dirq_get_granularity (dirq_t dirq)
{
  return(dirq->granularity);
}

/*
 * rndhex (always forced to 0..15)
 */

void dirq_set_rndhex (dirq_t dirq, int value)
{
  dirq->rndhex = (value < 0) ? ((-value) % 16) : (value % 16);
}

int dirq_get_rndhex (dirq_t dirq)
{
  return(dirq->rndhex);
}

/*
 * umask (no magic umask manipulation: what is given is stored as is)
 */

void dirq_set_umask (dirq_t dirq, mode_t value)
{
  dirq->umask = value;
}

mode_t dirq_get_umask (dirq_t dirq)
{
  return(dirq->umask);
}

/*
 * maxlock (assumed to be zero if negative)
 */

void dirq_set_maxlock (dirq_t dirq, int value)
{
  dirq->maxlock = (value < 0) ? 0 : value;
}

int dirq_get_maxlock (dirq_t dirq)
{
  return(dirq->maxlock);
}

/*
 * maxtemp (assumed to be zero if negative)
 */

void dirq_set_maxtemp (dirq_t dirq, int value)
{
  dirq->maxtemp = (value < 0) ? 0 : value;
}

int dirq_get_maxtemp (dirq_t dirq)
{
  return(dirq->maxtemp);
}
