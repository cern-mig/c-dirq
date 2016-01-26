/*+*****************************************************************************
*                                                                              *
* C dirq error support                                                         *
*                                                                              *
**-****************************************************************************/

/*
 * $Id: dirq_error.c,v 1.8 2013/04/24 10:43:04 c0ns Exp $
 */

/*
 * record an error (both code and string)
 */

static void error_set (dirq_t dirq, int errcode, const char *fmt, ...)
{
  va_list ap;

  assert(errcode != 0);
  dirq->errcode = errcode;
  va_start(ap, fmt);
  vsprintf(dirq->buffer + dirq->dirs_offset, fmt, ap);
  va_end(ap);
  /* also reset the iterator as we overwrote its buffers... */
  iter_reset(dirq);
}

/*
 * return the "current" error code
 */

int dirq_get_errcode (dirq_t dirq)
{
  return(dirq->errcode);
}

/*
 * return the "current" error string (or NULL if there is no error)
 */

const char *dirq_get_errstr (dirq_t dirq)
{
  return(dirq->errcode ? (dirq->buffer + dirq->dirs_offset) : NULL);
}

/*
 * clear the "current" error (in the hope the caller knows what he is doing...)
 */

void dirq_clear_error (dirq_t dirq)
{
  dirq->errcode = 0;
}
