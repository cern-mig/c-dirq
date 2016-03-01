/*+*****************************************************************************
*                                                                              *
* C dirq iteration support                                                     *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2016
 */

/*
 * types
 */

typedef int (*iter_cb)(dirq_t, const char *, int);

/*
 * constants
 */

#define DIRS_SIZE      8
#define ELTS_SIZE     16
#define SUFFIX_LENGTH  4

/*
 * macros
 */

#define DIRBUF(_d,_i) (_d->buffer + _d->dirs_offset + (_i) * DIRS_SIZE)
#define ELTBUF(_d,_i) (_d->buffer + _d->elts_offset + (_i) * ELTS_SIZE)

/*
 * reset the iterator (i.e. dirq_next() will return NULL)
 */

static void iter_reset (dirq_t dirq)
{
  dirq->dirs_index = dirq->dirs_count = 0;
  dirq->elts_index = dirq->elts_count = 0;
}

/*
 * check whether the given string is made only of hex digits
 */

static int _ishexstr (const char *cp, int len)
{
  while (len-- > 0) {
    if ((*cp < '0' || *cp > '9') && (*cp < 'a' || *cp > 'f'))
      return(0);
    cp++;
  }
  return(1);
}

/*
 * iterate over a directory (if it does not exist, it is assumed to be empty...)
 * the directory path is given as an offset to dirq->buffer because of realloc()
 */

static int _iterate (dirq_t dirq, int offset, iter_cb callback)
{
  DIR *dirp;
  struct dirent *dp;
  int result, total;

  total = 0;
  dirp = opendir(dirq->buffer + offset);
  if (!dirp) {
    if (errno == ENOENT)
      return(0);
    error_set(dirq, errno, "cannot opendir(%s): %s",
              dirq->buffer + offset, ERROR);
    return(-1);
  }
  while (1) {
    errno = 0;
    dp = readdir(dirp);
    if (!dp) {
      if (errno != 0) {
        error_set(dirq, errno, "cannot readdir(%s): %s",
                  dirq->buffer + offset, ERROR);
        return(-1);
      }
      /* end of directory */
      break;
    }
    if (dp->d_name[0] == '.') {
      if (dp->d_name[1] == '\0')
        continue;
      if (dp->d_name[1] == '.' && dp->d_name[2] == '\0')
        continue;
    }
    result = callback(dirq, dp->d_name, strlen(dp->d_name));
    if (result < 0) {
      (void) closedir(dirp); /* best effort cleanup... */
      return(result);
    }
    total += result;
  }
  if (closedir(dirp) < 0) {
    error_set(dirq, errno, "cannot closedir(%s): %s",
              dirq->buffer + offset, ERROR);
    return(-1);
  }
  return(total);
}

/*
 * get the list of intermediate directories
 */

static int _get_dirs_cmp (const void *elt1, const void *elt2)
{
  return(strncmp((const char *)elt1, (const char *)elt2, DIRS_SIZE));
}

static int _get_dirs_cb (dirq_t dirq, const char *name, int len)
{
  if (dirq->dirs_offset + (dirq->dirs_count + 1) * DIRS_SIZE >= dirq->allocated)
    allocate_more(dirq);
  if (len == DIR_NAME_LENGTH && _ishexstr(name, len)) {
    strncpy(DIRBUF(dirq,dirq->dirs_count), name, len);
    dirq->dirs_count++;
  }
  return(0);
}

static int _get_dirs (dirq_t dirq)
{
  int result;

  iter_reset(dirq);
  result = _iterate(dirq, 0, _get_dirs_cb);
  if (result < 0)
    return(result);
  if (dirq->dirs_count > 0)
    qsort(DIRBUF(dirq,0), dirq->dirs_count, DIRS_SIZE, _get_dirs_cmp);
  dirq->elts_offset = dirq->dirs_offset + dirq->dirs_count * DIRS_SIZE;
  return(0);
}

/*
 * get the list of elements (from the intermediate directory in tmp1)
 */

static int _get_elts_cmp (const void *elt1, const void *elt2)
{
  return(strcmp((const char *)elt1, (const char *)elt2));
}

static int _get_elts_cb (dirq_t dirq, const char *name, int len)
{
  if (dirq->elts_offset + (dirq->elts_count + 1) * ELTS_SIZE >= dirq->allocated)
    allocate_more(dirq);
  if (len == ELT_NAME_LENGTH && _ishexstr(name, len)) {
    strncpy(ELTBUF(dirq,dirq->elts_count), name, len);
    *(ELTBUF(dirq,dirq->elts_count) + len) = '\0';
    dirq->elts_count++;
  }
  return(0);
}

static int _get_elts (dirq_t dirq)
{
  int result;

  dirq->elts_index = dirq->elts_count = 0;
  result = _iterate(dirq, dirq->tmp1_offset, _get_elts_cb);
  if (result < 0)
    return(result);
  if (dirq->elts_count > 0)
    qsort(ELTBUF(dirq,0), dirq->elts_count, ELTS_SIZE, _get_elts_cmp);
  return(0);
}

/*
 * dirq_first(DIRQ): NAME | NULL end or error
 */

const char *dirq_first (dirq_t dirq)
{
  int result;

  result = _get_dirs(dirq);
  if (result < 0)
    return(NULL);
  return(dirq_next(dirq));
}

/*
 * dirq_next(DIRQ): NAME | NULL end or error
 */

const char *dirq_next (dirq_t dirq)
{
  int result;

  if (dirq->elts_index < dirq->elts_count) {
    assert(dirq->dirs_index > 0);
    strncpy(TMP1NAME(dirq), DIRBUF(dirq,dirq->dirs_index-1), DIRS_SIZE);
    *(TMP1NAME(dirq) + DIRS_SIZE) = '/';
    strcpy(TMP1NAME(dirq) + DIRS_SIZE + 1, ELTBUF(dirq,dirq->elts_index));
    dirq->elts_index++;
    return(TMP1NAME(dirq));
  }
  while (dirq->dirs_index < dirq->dirs_count) {
    strncpy(TMP1NAME(dirq), DIRBUF(dirq,dirq->dirs_index), DIRS_SIZE);
    *(TMP1NAME(dirq) + DIRS_SIZE) = '\0';
    result = _get_elts(dirq);
    if (result < 0)
      return(NULL);
    dirq->dirs_index++;
    if (dirq->elts_index < dirq->elts_count) {
      *(TMP1NAME(dirq) + DIRS_SIZE) = '/';
      strcpy(TMP1NAME(dirq) + DIRS_SIZE + 1, ELTBUF(dirq,dirq->elts_index));
      dirq->elts_index++;
      return(TMP1NAME(dirq));
    }
  }
  return(NULL);
}

/*
 * dirq_count(DIRQ): COUNT | -1 error
 */

int dirq_count (dirq_t dirq)
{
  int count, result;

  count = 0;
  result = _get_dirs(dirq);
  if (result < 0)
    return(-1);
  while (dirq->dirs_index < dirq->dirs_count) {
    strncpy(TMP1NAME(dirq), DIRBUF(dirq,dirq->dirs_index), DIRS_SIZE);
    *(TMP1NAME(dirq) + DIRS_SIZE) = '\0';
    result = _get_elts(dirq);
    if (result < 0)
      return(-1);
    count += dirq->elts_count;
    dirq->dirs_index++;
  }
  iter_reset(dirq); /* we have messed up with the iterator... */
  return(count);
}

/*
 * dir_purge(DIRQ): COUNT removals | -1 error
 */

/* locally abuse unused fields in the dirq object for purging... */
#define purge_maxlock elts_index
#define purge_maxtemp elts_offset

static int _purge_cb (dirq_t dirq, const char *name, int len)
{
  struct stat sb;

  if ((dirq->purge_maxlock != 0 || dirq->purge_maxtemp != 0) &&
      len >= SUFFIX_LENGTH && name[len - SUFFIX_LENGTH] == '.') {
    /* dot file to maybe remove... */
    strncpy(TMP2NAME(dirq) + DIRS_SIZE + 1, name, len);
    *(TMP2NAME(dirq) + DIRS_SIZE + 1 + len) = '\0';
    if (stat(TMP2BUF(dirq), &sb) != 0) {
      if (errno == ENOENT)
        return(0);
      error_set(dirq, errno, "cannot stat(%s): %s", TMP2BUF(dirq), ERROR);
      return(-1);
    }
    if (dirq->purge_maxlock != 0 &&
        strcmp(&name[len - SUFFIX_LENGTH], LOCKED_SUFFIX) == 0 &&
        sb.st_mtime < dirq->purge_maxlock) {
      if (unlink(TMP2BUF(dirq)) != 0) {
        if (errno == ENOENT)
          return(0);
        error_set(dirq, errno, "cannot unlink(%s): %s", TMP2BUF(dirq), ERROR);
        return(-1);
      }
      return(1);
    }
    if (dirq->purge_maxtemp != 0 &&
        strcmp(&name[len - SUFFIX_LENGTH], TEMPORARY_SUFFIX) == 0 &&
        sb.st_mtime < dirq->purge_maxtemp) {
      if (unlink(TMP2BUF(dirq)) != 0) {
        if (errno == ENOENT)
          return(0);
        error_set(dirq, errno, "cannot unlink(%s): %s", TMP2BUF(dirq), ERROR);
        return(-1);
      }
      return(1);
    }
  }
  /* count it to prevent parent directory removal */
  dirq->elts_count++;
  return(0);
}

int dirq_purge (dirq_t dirq)
{
  int count, result;
  uint32_t now;

  count = 0;
  result = _get_dirs(dirq);
  if (result < 0)
    return(-1);
  now = (uint32_t)time(NULL);
  dirq->purge_maxlock = dirq->maxlock ? (now - dirq->maxlock) : 0;
  dirq->purge_maxtemp = dirq->maxtemp ? (now - dirq->maxtemp) : 0;
  while (dirq->dirs_index < dirq->dirs_count) {
    strncpy(TMP1NAME(dirq), DIRBUF(dirq,dirq->dirs_index), DIRS_SIZE);
    *(TMP1NAME(dirq) + DIRS_SIZE) = '\0';
    strncpy(TMP2NAME(dirq), DIRBUF(dirq,dirq->dirs_index), DIRS_SIZE);
    *(TMP2NAME(dirq) + DIRS_SIZE) = '/';
    dirq->elts_count = 0;
    result = _iterate(dirq, dirq->tmp1_offset, _purge_cb);
    if (result < 0)
      return(-1);
    count += result;
    if (dirq->elts_count == 0 && dirq->dirs_index < dirq->dirs_count - 1) {
      /* remove empty intermediate directories except the last one */
      if (rmdir(TMP1BUF(dirq)) != 0 && errno != ENOENT) {
        error_set(dirq, errno, "cannot rmdir(%s): %s", TMP1BUF(dirq), ERROR);
        return(-1);
      }
      count++;
    }
    dirq->dirs_index++;
  }
  iter_reset(dirq); /* we have messed up with the iterator... */
  return(count);
}

#undef purge_maxlock
#undef purge_maxtemp
