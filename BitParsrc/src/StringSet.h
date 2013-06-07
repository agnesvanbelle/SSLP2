
/*MA****************************************************************/
/*                                                                 */
/*     File: StringSet.h                                           */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Wed Sep 24 14:10:25 2008 (schmid)                     */
/* Modified: Thu Jan 14 16:32:20 2010 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#ifndef __STRING_SET
#define __STRING_SET

#include <string.h>
#include "sgi.h"


/*****************  class StringSet  *****************************/

class StringSet {

  // problem: allocated memory is not freed by the destructor

private:

  struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
      return strcmp(s1, s2) == 0;
    }
  };

  typedef hash_set<const char*, hash<const char*>, eqstr> SM;

  SM sm;

public:

  ~StringSet() {
    
  }

  typedef SM::iterator iterator;

  iterator find( const char *s ) { return sm.find(s); }
  iterator begin() { return sm.begin(); }
  iterator end()   { return sm.end(); }
  size_t   size()  { return sm.size(); }

  const char *operator()( const char *s ) {
    if (s == NULL)
      return s;
    iterator it=sm.find(s);
    if (it != sm.end())
      return *it;
    s = strdup(s);
    sm.insert(s);
    return s;
  }

  const char *lookup( const char *s ) {
    if (s == NULL)
      return s;
    iterator it=sm.find(s);
    if (it == sm.end())
      return NULL;
    return *it;
  }
};

#endif
