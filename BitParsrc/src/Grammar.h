/*******************************************************************/
/*      File: Grammar.h                                            */
/*    Author: Helmut Schmid                                        */
/*   Purpose:                                                      */
/*   Created: Tue Oct 29 10:01:36 2002                             */
/*  Modified: Wed May  4 13:37:59 2011 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/
#include <stdio.h>

#include <assert.h>
#include <vector>
using std::vector;

#include "sgi.h"
#include "bitop.h"
#include "traces.h"
#include "prob.h"

extern bool Quiet;
extern bool Verbose;
extern bool WithHeads;
extern bool WithTraces;

typedef int RuleNumber;
typedef enum { left, right } HeadDir;

class MRule;


/**********************  class Rule  *******************************/

class Rule {

 private:
  unsigned short l;
  SymNum *symnum;
  // Problem: space allocated for symnum is not freed
  // Advantage: rules can be copied efficiently by copying the
  // pointer symnum rather than reallocating a new array and copying
  // the array contents

 public:
  unsigned short headpos;
  Rule( vector<int> symbols, unsigned short hp=0 ) : headpos(hp) {
    l = (unsigned short)symbols.size();
    symnum = new SymNum[l];
    for( int i=0; i<l; i++ )
      symnum[i] = symbols[i];
  };

  // rule copy operator; symbol array is not duplicated
  Rule( const Rule &r ) { l = r.l; symnum = r.symnum; headpos = r.headpos; };

  unsigned short length() const { return (int) l; };

  int symbol( size_t n ) const { 
    if (n < l)
      return symnum[n];
    assert(0);
    throw "in function Rule::symbol(int)";
  };
};


/*********************** class Grammar *****************************/

class Grammar {

 public:
  SymbolTable symbols;
  SymNum startsym;
  vector<Rule> rules;
  vector<Prob> ruleprob;
  vector<double> rulefreq;
  Traces traces;

  Grammar( FILE*, char *ss=NULL, char tss='*', char tes='*' );

  const char *symbol_name( int n ) { return symbols.name(n); }

  size_t number_of_symbols() { return symbols.size(); }

  SymNum symnum( const char *s ) { return symbols.number(s); }

  int symbol_number( const char *s ) { 
    SymbolTable::iterator it=symbols.find(s);
    if (it == symbols.end())
      return -1;
    return it->second;
  }

  SymNum start_symbol() { return startsym; };

  void print_rule( int n, FILE *file=stdout ) {
    fprintf(file, "%s ->", symbol_name(rules[n].symbol(0)));
    for( int i=1; i<rules[n].length(); i++ )
      fprintf(file," %s", symbol_name(rules[n].symbol(i)));
    fprintf(file, " (%g)", (double)ruleprob[n]);
    fputc('\n', file);
  }

  void print( FILE *file=stdout ) {
    for( size_t i=0; i<rules.size(); i++ )
      print_rule( (int)i, file );
  }

  void incr_freq( int n, double f ) { rulefreq[n] += f; };

  void estimate_probs( vector<double> &tagfreq, vector<double> &termprob );

  void store( FILE *file );

  friend class Lexicon;
};


/*********************** class NFRule  *****************************/

class NFRule {

 public:
  SymNum left, right;
  HeadDir headdir;
  RuleNumber source_rule;

  NFRule( int l, int r, HeadDir h, RuleNumber sr=-1 ) 
    : left(l), right(r), headdir(h), source_rule(sr) {}

  bool is_aux() { return source_rule == -1; }
};


/************************* class ChainRule *************************/

class ChainRule {
  public:
  RuleNumber source_rule;
  SymNum symbol;
  ChainRule( RuleNumber sr, SymNum s ) : source_rule(sr), symbol(s) {}
};


/************************* class Chain *****************************/

class Chain {

 public:

  vector<ChainRule> up;
  vector<ChainRule> down;
  unsigned long *bitvec;

  Chain() { bitvec=NULL; }

  ~Chain() {
    delete[] bitvec;
  }

  friend class NFGrammar;
};


/*********************** class NFGrammar ***************************/

class NFGrammar {

 private:
  Grammar &g;

  vector<char*> name;
  vector< vector<NFRule> > rules_for_sym;

  void create_bit_maps( vector<MRule> &chainrule );
  void create_nfg();

 public:
  size_t bitvec_length;  // length of bitvectors
  vector<Chain> chain;

  NFGrammar( Grammar &gram ) : g(gram) { create_nfg(); };

  size_t number_of_symbols() { return name.size(); };

  const char *symbol_name( int n ) { return name[n]; };

  vector<NFRule> &get_rules( int sn ) { return rules_for_sym[sn]; };

  Prob ruleprob( RuleNumber rn ) {
    if (rn == -1)
      return (Prob)1.0; // auxiliary rule
    return g.ruleprob[rn];
  }

  ~NFGrammar() {
    for( size_t i=0; i<name.size(); i++ )
      free( (char*)name[i] );
  }
};
