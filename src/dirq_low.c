/*+*****************************************************************************
*                                                                              *
* C dirq low level utilities                                                   *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2017
 */

/*
 * print an error message on stderr and exit
 */

static void die (const char *fmt, ...)
{
  va_list ap;

  fputs("*** ", stderr);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputs("\n", stderr);
  exit(-1);
}

/*
 * "safely" allocate memory (i.e. die on error)
 */

static void *safe_malloc (size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    die("cannot malloc(%d): %s", size, ERROR);
  return(ptr);
}

/*
 * "safely" reallocate memory (i.e. die on error)
 */

static void *safe_realloc (void *oldptr, size_t size)
{
  void *newptr;

  newptr = realloc(oldptr, size);
  if (!newptr)
    die("cannot realloc(%d): %s", size, ERROR);
  return(newptr);
}
