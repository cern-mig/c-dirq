/*+*****************************************************************************
*                                                                              *
* C dirq test program                                                          *
*                                                                              *
**-****************************************************************************/

/*
 * Author: Lionel Cons (http://cern.ch/lionel.cons)
 * Copyright (C) CERN 2012-2024
 */

/*
 * include
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "dirq.h"

/*
 * macros
 */

#define ERROR strerror(errno)
#define UNUSED(_x) do { (void)(_x); } while (0)

/*
 * global variables
 */

char *ProgramName = NULL;
dirq_t DirQ = NULL;
struct timespec Start, Stop;
double Elapsed = 0;

char *Buffer;
size_t BufOffset, BufLength;

/*
 * options
 */

struct option Options[] = {
  { "count",       required_argument, 0, 'c' },
  { "debug",       no_argument,       0, 'd' },
  { "granularity", required_argument, 0,  0  },
  { "header",      no_argument,       0,  0  },
  { "help",        no_argument,       0, 'h' },
  { "list",        no_argument,       0, 'l' },
  { "manual",      no_argument,       0, 'm' },
  { "maxlock",     required_argument, 0,  0  },
  { "maxtemp",     required_argument, 0,  0  },
  { "path",        required_argument, 0, 'p' },
  { "random",      no_argument,       0, 'r' },
  { "size",        required_argument, 0,  0  },
  { "sleep",       required_argument, 0,  0  },
  { "type",        required_argument, 0,  0  },
  { "umask",       required_argument, 0,  0  },
  { NULL,          0,                 0,  0  }
};

int     OptCount       = 0;
int     OptDebug       = 0;
int     OptGranularity = 0;
int     OptHeader      = 0;
int     OptMaxLock     = 0;
int     OptMaxTemp     = 0;
char   *OptPath        = NULL;
int     OptRandom      = 0;
int     OptSize        = 0;
double  OptSleep       = 0;
char   *OptType        = "simple";
int     OptUmask       = 0;

/*
 * constants
 */

#define DO_GET     0
#define DO_ITERATE 1
#define DO_REMOVE  2

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
 * test helpers
 */

static void debug (int timing, const char *fmt, ...)
{
  va_list ap;
  time_t now;
  struct tm *stmp;
  char buffer[32];

  if (!OptDebug)
    return;
  now = time(NULL);
  stmp = localtime(&now);
  strftime(buffer, sizeof(buffer), "%Y/%m/%d-%H:%M:%S", stmp);
  fprintf(stderr, "# %s %s[%d] ", buffer, ProgramName, getpid());
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  if (timing)
    fprintf(stderr, " in %.3f seconds", Elapsed);
  fputs("\n", stderr);
}

static void setup (void)
{
  const char *errstr;

  DirQ = dirq_new(OptPath);
  errstr = dirq_get_errstr(DirQ);
  if (errstr != NULL)
    die("queue creation failed: %s", errstr);
  if (OptGranularity)
    dirq_set_granularity(DirQ, OptGranularity);
  if (OptMaxLock)
    dirq_set_maxlock(DirQ, OptMaxLock);
  if (OptMaxTemp)
    dirq_set_maxlock(DirQ, OptMaxTemp);
  if (OptUmask)
    dirq_set_umask(DirQ, OptUmask);
  dirq_now(DirQ, &Start);
}

static void cleanup (void)
{
  dirq_now(DirQ, &Stop);
  Elapsed = Stop.tv_sec - Start.tv_sec + (Stop.tv_nsec - Start.tv_nsec) / 1e9;
  dirq_free(DirQ);
}

static int safe_lock (const char *name)
{
  int result;
  const char *errstr;

  result = dirq_lock(DirQ, name, 1);
  if (result > 0)
    return(0);
  if (result != 0) {
    errstr = dirq_get_errstr(DirQ);
    assert(errstr != NULL);
    die("locking failed: %s", errstr);
  }
  return(1);
}

static void safe_unlock (const char *name)
{
  int result;
  const char *errstr;

  result = dirq_unlock(DirQ, name, 0);
  if (result != 0) {
    errstr = dirq_get_errstr(DirQ);
    assert(errstr != NULL);
    die("unlocking failed: %s", errstr);
  }
}

static void safe_remove (const char *name)
{
  int result;
  const char *errstr;

  result = dirq_remove(DirQ, name);
  if (result != 0) {
    errstr = dirq_get_errstr(DirQ);
    assert(errstr != NULL);
    die("removing failed: %s", errstr);
  }
}

static int noop (dirq_t dirq, const char *buffer, size_t length)
{
  UNUSED(dirq);
  UNUSED(buffer);
  UNUSED(length);
  return(0);
}

static void safe_get (const char *name)
{
  int result;
  const char *errstr;

  result = dirq_get(DirQ, name, noop);
  if (result != 0) {
    errstr = dirq_get_errstr(DirQ);
    assert(errstr != NULL);
    die("getting failed: %s", errstr);
  }
}

/*
 * info test
 */

static void test_info (void)
{
  debug(0, "using dirq version %d.%d (0x%02x)",
        DIRQ_VERSION_MAJOR, DIRQ_VERSION_MINOR, DIRQ_VERSION_HEX);
}

/*
 * count test
 */

static void test_count (void)
{
  int count;
  const char *errstr;

  setup();
  count = dirq_count(DirQ);
  if (count < 0) {
      errstr = dirq_get_errstr(DirQ);
      assert(errstr != NULL);
      die("counting failed: %s", errstr);
  }
  cleanup();
  debug(1, "queue has %d elements", count);
}

/*
 * add test
 */

static void new_element (int count)
{
  int i, length;
  double max, rnd;

  if (OptSize > 0) {
    if (OptRandom) {
      /* see Irwin-Hall in http://en.wikipedia.org/wiki/Normal_distribution */
      max = (double) RAND_MAX + 1.0;
      rnd = rand() / max + rand() / max + rand() / max + rand() / max
          + rand() / max + rand() / max + rand() / max + rand() / max
          + rand() / max + rand() / max + rand() / max + rand() / max;
      rnd -= 6.0;
      rnd *= OptSize / 6.0;
      rnd += OptSize + 0.5;
      length = (int) rnd;
    } else {
      length = OptSize;
    }
    for (i=0; i<length; i++)
      Buffer[i] = 32 + (rand() % 95);
    BufLength = length;
  } else if (OptSize < 0) {
    BufLength = 0;
  } else {
    sprintf(Buffer, "Element %d ;-)\n", count);
    BufLength = strlen(Buffer);
  }
}

static int test_add_iow (dirq_t dirq, char *buffer, size_t length)
{
  size_t chunk;

  UNUSED(dirq);
  chunk = BufLength - BufOffset;
  if (chunk > 0) {
    if (chunk > length)
      chunk = length;
    strncpy(buffer, &Buffer[BufOffset], chunk);
    BufOffset += chunk;
  }
  return(chunk);
}

static void test_add (void)
{
  int i;
  const char *name, *errstr;

  debug(0, "adding %d elements to the queue...", OptCount);
  setup();
  for (i=0; i<OptCount; i++) {
    new_element(i);
    BufOffset = 0;
    name = dirq_add(DirQ, test_add_iow);
    if (name == NULL) {
      errstr = dirq_get_errstr(DirQ);
      assert(errstr != NULL);
      die("adding failed: %s", errstr);
    }
    if (OptDebug > 1)
      debug(0, "added element %s", name);
  }
  cleanup();
  debug(1, "added %d elements", OptCount);
}

/*
 * remove test
 */

static void test_remove (void)
{
  const char *name, *errstr;
  int count;

  debug(0, "removing %d elements from the queue...", OptCount);
  setup();
  count = 0;
  while (1) {
    for (name=dirq_first(DirQ); name; name=dirq_next(DirQ)) {
      if (OptDebug > 1)
        debug(0, "seen element %s", name);
      if (safe_lock(name)) {
        safe_remove(name);
        count++;
        if (count >= OptCount)
          break;
      }
    }
    if (count >= OptCount)
      break;
    errstr = dirq_get_errstr(DirQ);
    if (errstr)
      die("iteration failed: %s", errstr);
  }
  cleanup();
  debug(1, "removed %d elements", count);
}

/*
 * one pass test (lock+(get|remove)?+unlock)
 */

static void test_iterate (int what)
{
  const char *name, *errstr;
  int count;

  switch (what) {
  case DO_GET:
    name = "getting";
    break;
  case DO_ITERATE:
    name = "iterating";
    break;
  case DO_REMOVE:
    name = "removing";
    break;
  default:
    abort();
  }
  debug(0, "%s all elements in the queue (one pass)...", name);
  setup();
  count = 0;
  for (name=dirq_first(DirQ); name; name=dirq_next(DirQ)) {
    if (OptDebug > 1)
      debug(0, "seen element %s", name);
    if (safe_lock(name)) {
      if (what == DO_GET)
        safe_get(name);
      count++;
      if (what == DO_REMOVE)
        safe_remove(name);
      else
        safe_unlock(name);
    }
  }
  errstr = dirq_get_errstr(DirQ);
  if (errstr)
    die("iteration failed: %s", errstr);
  cleanup();
  switch (what) {
  case DO_GET:
    name = "got";
    break;
  case DO_ITERATE:
    name = "iterated";
    break;
  case DO_REMOVE:
    name = "removed";
    break;
  default:
    abort();
  }
  debug(1, "%s %d elements", name, count);
}

/*
 * size test
 */

static void test_size (void)
{
  FILE *fp;
  int status, kb;

  if (strchr(OptPath, '\'') || strchr(OptPath, '\\'))
    die("unexpected character in path: %s", OptPath);
  sprintf(Buffer, "du -ks %s", OptPath);
  fp = popen(Buffer, "r");
  if (!fp)
    die("cannot execute: %s", Buffer);
  if (!fgets(Buffer, 1024, fp))
    die("du command failed: %s", ERROR);
  status = pclose(fp);
  if (status != 0)
    die("du command failed: %d", status);
  if (sscanf(Buffer, "%d ", &kb) != 1)
    die("unexpected output: %s", Buffer);
  debug(0, "queue uses %d KB", kb);
}

/*
 * purge test
 */

static void test_purge (void)
{
  int count;
  const char *errstr;

  setup();
  count = dirq_purge(DirQ);
  if (count < 0) {
      errstr = dirq_get_errstr(DirQ);
      assert(errstr != NULL);
      die("purging failed: %s", errstr);
  }
  cleanup();
  debug(1, "purged %d elements or directories", count);
}

/*
 * simple meta-test (only for non=existing path!)
 */

static void test_simple (void)
{
  struct stat sb;
  DIR *dirp;
  struct dirent *dp;
  char dirname[32];

  if (!OptCount)
    die("missing option: --count");
  assert(strlen(OptPath) < 980);
  if (stat(OptPath, &sb) != 0) {
    if (errno != ENOENT)
      die("cannot stat(%s): %s", OptPath, ERROR);
  } else {
    die("cannot use existing path for simple test: %s", OptPath);
  }
  test_info();
  test_add();
  test_count();
  test_size();
  test_purge();
  test_iterate(DO_GET);
  test_remove();
  test_purge();
  dirname[0] = '\0';
  dirp = opendir(OptPath);
  if (!dirp)
      die("cannot opendir(%s): %s", OptPath, ERROR);
  while (1) {
    dp = readdir(dirp);
    if (!dp)
      break;
    if (dp->d_name[0] == '.') {
      if (dp->d_name[1] == '\0')
        continue;
      if (dp->d_name[1] == '.' && dp->d_name[2] == '\0')
        continue;
    }
    if (dirname[0])
      die("unexpected directory: %s", dp->d_name);
    assert(strlen(dp->d_name) == 8);
    strcpy(dirname, dp->d_name);
  }
  if (closedir(dirp) < 0)
      die("cannot closedir(%s): %s", OptPath, ERROR);
  if (!dirname[0])
    die("missing sub-directory");
  sprintf(Buffer, "%s/%s", OptPath, dirname);
  if (rmdir(Buffer) != 0)
    die("cannot rmdir(%s): %s", Buffer, ERROR);
  if (rmdir(OptPath) != 0)
    die("cannot rmdir(%s): %s", OptPath, ERROR);
  debug(0, "finished simple test successfully");
}

/*
 * usage
 */

static void usage (int status)
{
  int i;

  printf("Usage: %s [OPTIONS] [NAME]\n", ProgramName);
  printf("Options:\n");
  for (i=0; Options[i].name; i++) {
    printf("\t");
    if (Options[i].val)
      printf("-%c|", Options[i].val);
    else
      printf("   ");
    printf("--%s", Options[i].name);
    if (Options[i].has_arg == required_argument)
      printf("=VALUE");
    printf("\n");
  }
  exit(status);
}

/*
 * main
 */

int main(int argc, char *argv[])
{
  char *cp;
  int optc, opti, length;

  cp = strrchr(argv[0], '/');
  ProgramName = cp ? (cp + 1) : argv[0];
  while (1) {
    opti = 0;
    optc = getopt_long(argc, argv, "c:dhlmp:r", Options, &opti);
    if (optc == -1)
      break;
    switch (optc) {
    case 'c':
      OptCount = atoi(optarg);
      break;
    case 'd':
      OptDebug++;
      break;
    case 'h':
    case 'm':
      usage(0);
      break;
    case 'l':
      printf("Available tests: %s\n",
             "add count get info iterate purge remove simple size");
      exit(0);
      break;
    case 'p':
      OptPath = optarg;
      break;
    case 'r':
      OptRandom++;
      break;
    case 0:
      if (strcmp(Options[opti].name, "granularity") == 0)
        OptGranularity = atoi(optarg);
      else if (strcmp(Options[opti].name, "header") == 0)
        OptHeader++;
      else if (strcmp(Options[opti].name, "maxlock") == 0)
        OptMaxLock = atoi(optarg);
      else if (strcmp(Options[opti].name, "maxtemp") == 0)
        OptMaxTemp = atoi(optarg);
      else if (strcmp(Options[opti].name, "size") == 0)
        OptSize = atoi(optarg);
      else if (strcmp(Options[opti].name, "sleep") == 0)
        OptSleep = atof(optarg);
      else if (strcmp(Options[opti].name, "type") == 0)
        OptType = optarg;
      else if (strcmp(Options[opti].name, "umask") == 0)
        OptUmask = atoi(optarg);
      else
        abort();
      break;
    default:
      usage(1);
    }
  }
  if (strcmp(OptType, "simple") != 0)
    die("unsupported dirq type: %s", OptType);
  if (optind + 1 != argc)
    usage(1);
  if (OptRandom && OptSize <= 0)
    die("missing option (with --random): --size");
  if (!OptPath)
    die("missing option: --path");
  length = OptSize > 1024 ? OptSize : 1024;
  if (OptRandom)
    length *= 2;
  Buffer = malloc(length);
  if (!Buffer)
    die("cannot allocate %d bytes!", length);
  if (OptSize > 0)
    srand(getpid());
  if (OptSleep > 0)
    usleep(OptSleep * 1e6);
  if (strcmp(argv[optind], "add") == 0) {
    test_add();
  } else if (strcmp(argv[optind], "count") == 0) {
    test_count();
  } else if (strcmp(argv[optind], "get") == 0) {
    test_iterate(DO_GET);
  } else if (strcmp(argv[optind], "info") == 0) {
    test_info();
  } else if (strcmp(argv[optind], "iterate") == 0) {
    test_iterate(DO_ITERATE);
  } else if (strcmp(argv[optind], "purge") == 0) {
    test_purge();
  } else if (strcmp(argv[optind], "remove") == 0) {
    if (OptCount == 0)
      test_iterate(DO_REMOVE);
    else
      test_remove();
  } else if (strcmp(argv[optind], "simple") == 0) {
    test_simple();
  } else if (strcmp(argv[optind], "size") == 0) {
    test_size();
  } else {
    die("unknown test: %s", argv[optind]);
  }
  free(Buffer);
  exit(0);
}
