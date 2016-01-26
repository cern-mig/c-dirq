/*+*****************************************************************************
*                                                                              *
* C dirq miscellaneous support                                                 *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2016
 */

/*
 * allocate more memory in a directory queue object
 */

static void allocate_more (dirq_t dirq)
{
  dirq->allocated += 8192;
  dirq->buffer = (char *)safe_realloc((void *)dirq->buffer, dirq->allocated);
}

/*
 * make sure a directory exists, non recursively
 */

static int ensure_directory (dirq_t dirq, const char *path)
{
  struct stat sb;

 same_player_shoot_again:
  if (stat(path, &sb) != 0) {
    if (errno != ENOENT) {
      error_set(dirq, errno, "cannot stat(%s): %s", path, ERROR);
      return(-1);
    }
  } else {
    if (S_ISDIR(sb.st_mode)) {
      return(0);
    } else {
      error_set(dirq, ENOTDIR, "cannot mkdir(%s, 0777): %s", path,
                strerror(ENOTDIR));
      return(-1);
    }
  }
  if (mkdir(path, 0777) != 0) {
    if (errno != EEXIST) {
      error_set(dirq, errno, "cannot mkdir(%s, 0777): %s", path, ERROR);
      return(-1);
    } else {
      goto same_player_shoot_again;
    }
  }
  return(0);
}

/*
 * make sure a directory exists, recursively
 */

static int ensure_directory_recursively (dirq_t dirq, const char *path)
{
  size_t len;
  char tmp[MAXPATHLEN];
  char *cp;
  int result;

  len = strlen(path);
  assert(len < MAXPATHLEN);
  strcpy(tmp, path);
  while (len > 1 && tmp[len-1] == '/') {
    len--;
    tmp[len] = '\0';
  }
  for (cp=tmp+1; *cp; cp++) {
    if (*cp == '/') {
      *cp = '\0';
      result = ensure_directory(dirq, tmp);
      if (result != 0)
        return(result);
      *cp = '/';
    }
  }
  return(ensure_directory(dirq, tmp));
}

/*
 * set the name of a new element in a temporary path buffer
 */

static void set_new_name (dirq_t dirq, int offset)
{
  struct timespec ts;

  dirq_now(dirq, &ts);
  sprintf(dirq->buffer + offset + dirq->pathlen + 10, "%08x%05x%01x",
          (uint32_t)ts.tv_sec,
          (uint32_t)(ts.tv_nsec / 1000),
          (uint32_t)dirq->rndhex);
}

/*
 * set the name of the intermediate directory to use and make sure it exists
 * (put it in _both_ temporary path buffers, with a trailing slash)
 */

static int set_insertion_directory (dirq_t dirq)
{
  uint32_t now;
  int result;

  now = (uint32_t)time(NULL);
  if (dirq->granularity)
    now -= now % dirq->granularity;
  sprintf(TMP1NAME(dirq), "%08x", now);
  result = ensure_directory(dirq, TMP1BUF(dirq));
  if (result != 0)
    return(result);
  *(TMP1NAME(dirq) + DIR_NAME_LENGTH) = '/';
  strncpy(TMP2NAME(dirq), TMP1NAME(dirq), DIR_NAME_LENGTH + 1);
  return(0);
}

/*
 * add the given temporary path to the directory queue
 */

static int add_temporary_path (dirq_t dirq, const char *path)
{
  while (1) {
    set_new_name(dirq, dirq->tmp2_offset);
    if (link(path, TMP2BUF(dirq)) == 0) {
      if (unlink(path) == 0) {
        return(0);
      } else {
        error_set(dirq, errno, "cannot unlink(%s): %s", path, ERROR);
        return(-1);
      }
    } else {
      if (errno != EEXIST) {
        error_set(dirq, errno, "cannot link(%s, %s): %s", path, TMP2BUF(dirq),
                  ERROR);
        return(-1);
      }
    }
  }
}
