
/*MA****************************************************************/
/*                                                                 */
/*     File: traces.h                                              */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jan  2 14:55:10 2003                              */
/* Modified: Mon Feb 23 09:45:59 2009 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include "SymbolTable.h"


/*****************  class Traces  **********************************/

class Trace {
 public:
  int pos;
  SymNum sn;
  Trace( int p, SymNum n ) { pos = p;  sn = n; };
};


/*****************  class Traces  **********************************/

class Traces {

private:
  
  SymbolTable ST;

  vector< vector<Trace> > traces;

public:
  char start_symbol, end_symbol;

  Traces() { start_symbol = end_symbol = '*'; };

  void add_trace( size_t rule_number, int pos, char *s ) {
    SymNum n=ST.number(s);
    if (rule_number >= traces.size())
      traces.resize(rule_number+1);
    traces[rule_number].push_back(Trace(pos-1, n));
  }

  vector<Trace> &get( size_t rule_number ) {
    if (rule_number >= traces.size())
      traces.resize(rule_number+1);
    return traces[rule_number];
  }

  const char *symbol_name( int n ) { return ST.name(n); };

  bool is_trace( const char *s )
    { return s[0] == start_symbol && s[strlen(s)-1] == end_symbol; }

  void print_trace( int rn, size_t &tpos, size_t dpos, FILE *file );

};

