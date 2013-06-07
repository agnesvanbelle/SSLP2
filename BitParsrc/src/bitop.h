
/*******************************************************************/
/*                                                                 */
/*     File: bitop.h                                               */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Feb 13 15:04:24 2004                              */
/* Modified: Tue Apr 30 17:47:31 2013 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#include <stdlib.h>
#include <vector>
using std::vector;

const size_t ULONG_BITS = sizeof(unsigned long) * 8;

inline size_t bitarray_size( size_t n ) {
  return (n + ULONG_BITS - 1) / ULONG_BITS;
};

inline bool get_bit( unsigned long *bitvec, size_t pos ) {
  size_t x = pos / ULONG_BITS;
  size_t y = pos % ULONG_BITS;
  return bitvec[x] & ((unsigned long)1 << y);
}

inline void set_bit( unsigned long *bitvec, size_t pos ) {
  size_t x = pos / ULONG_BITS;
  size_t y = pos % ULONG_BITS;
  bitvec[x] |= (unsigned long)1 << y;
}

void extract_bits(size_t b, size_t e, unsigned long *bitvec, vector<size_t> &m);

