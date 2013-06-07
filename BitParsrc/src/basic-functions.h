
/*MA****************************************************************/
/*                                                                 */
/*     File: /home/users2/schmid/src/BitPar/basic-functions.h      */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Feb 14 12:13:56 2003                              */
/* Modified: Wed Jun  7 15:43:06 2006 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include <ctype.h>


bool empty_line( const char *s ) {
  while (isspace(*s)) s++;
  return *s==0;
}


#define INPUT( var ) \
  if (fread( &(var), sizeof var, 1, file) != 1) {\
    assert(0); throw("in input file"); }

#define INPUT1( var, count ) \
  if (fread( var, sizeof *var, (size_t)count, file) != (size_t)count) {\
    assert(0); throw("in input file"); }

#define OUTPUT( var ) \
  if (fwrite( &(var), sizeof var, 1, file) != 1) {\
    assert(0); throw("in output file"); }

#define OUTPUT1( var, count )                              \
  if (fwrite( var, sizeof *var, (size_t)count, file) != (size_t)count) {\
    assert(0); throw("in output file"); }

#define OUTPUTS( text )    \
  OUTPUT1(text, strlen(text)+1)

