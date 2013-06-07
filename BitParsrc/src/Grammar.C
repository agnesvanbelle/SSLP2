/*******************************************************************/
/*      file: /home/helmut/src/BitPar/Grammar.C                    */
/*    Author: Helmut Schmid                                        */
/*   Purpose:                                                      */
/*   Created: Tue Oct 29 10:15:44 2002                             */
/*  Modified: Tue Apr 30 17:51:24 2013 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include <assert.h>

#include <iostream>
using std::cerr;

#include "basic-functions.h"
#include "Grammar.h"

bool Quiet=false;
bool Verbose=false;
bool WithTraces=false;
bool WithHeads=false;


/*********************** class MRule *******************************/

class MRule : public Rule {

public:
  RuleNumber source_rule;

  MRule( vector<int> symbols, RuleNumber sr, unsigned short hp ) 
    : Rule(symbols, hp), source_rule(sr) {}

  MRule( Rule &r, RuleNumber sr ) 
    : Rule (r), source_rule(sr) {}
};


/*********************** class SymPair *****************************/

class SymPair {
  
public:
  SymNum first;
  SymNum second;
  HeadDir headdir;
  
  SymPair(): first(0), second(0), headdir(left) {};
  SymPair( SymNum c1, SymNum c2, HeadDir d=left )
    : first(c1), second(c2), headdir(d) {}
};

struct eqf {
  bool operator()(const SymPair &p1, const SymPair &p2) const {
    return (p1.first == p2.first && p1.second == p2.second &&
	    p1.headdir == p2.headdir);
  }
};

struct hashf {
  size_t operator()(const SymPair &p) const { 
    return ((p.first << 16) ^ (p.second << 15) ^ p.headdir);
  }
};


/*********************** class SymPairFreq *************************/

class SymPairFreq {

  typedef hash_map<const SymPair, size_t, hashf, eqf> SPF;

  SPF spf;

public:

  typedef SPF::iterator iterator;

  size_t &operator[]( SymPair p ) {
    iterator it=spf.find(p);
    if (it == end())
      return spf.insert(SPF::value_type(p, 0)).first->second;
    return it->second;
  };

  iterator begin() { return spf.begin(); }
  iterator end()   { return spf.end(); }
  size_t   size()  { return spf.size();  };
  void erase( SymPair &p ) { spf.erase(p); };
};


typedef hash_map<const SymPair, SymNum, hashf, eqf> SymPairNum;

#define BUFFER_SIZE 10000


/*******************************************************************/
/*                                                                 */
/*  Grammar::Grammar                                               */
/*                                                                 */
/*******************************************************************/

Grammar::Grammar( FILE *file, char *ss, char tss, char tes )

{
  char buffer[BUFFER_SIZE];

  traces.start_symbol = tss;
  traces.end_symbol = tes;
  if (!Quiet)
    cerr << "reading the grammar...";

  // Read the next rule
  for( size_t N=1; fgets(buffer, BUFFER_SIZE, file) != NULL; N++ ) {
    if (empty_line(buffer))
      continue;
    char *s, *p = buffer;
    vector<int> symbols;
    
    // scanning of the rule frequency
    double f=strtod(buffer, &p);
    if (p == buffer) {
      char *message=(char*)malloc(1000);
      sprintf(message, "in line %d of grammar file: missing rule frequency", (int)N);
      throw message;
    }
    rulefreq.push_back(f);

    // scanning of the symbols
    unsigned short headpos = 0;
    while ((s = strtok(p, " \t\n")) != NULL) {
      if (WithHeads && s[0] == '^') {
	s++;
	if (symbols.size() == 0)
	  fprintf(stderr, "Warning: in line %d of grammar file: head marking on parent node of rule", (int)N);
	if (headpos != 0)
	  fprintf(stderr, "Warning: in line %d of grammar file: multiple head markings in rule", (int)N);
	headpos = (unsigned short)symbols.size();
      }
      if (WithTraces && traces.is_trace(s)) {
	if (headpos == symbols.size()) {
	  char *message=(char*)malloc(1000);
	  sprintf(message, "in line %d of grammar file: head-marking at trace node: %s", (int)N, s);
	  throw message;
	}
	traces.add_trace(rules.size(), (int)symbols.size(), s);
      }
      else
	symbols.push_back(symnum(s));
      p = NULL;
    }
    if (symbols.size() > 0) {
      if (symbols.size() == 1) {
	char *message=(char*)malloc(1000);
	sprintf(message, "in line %d of grammar file: rule generates the empty string: %s", (int)N, buffer);
	throw message;
      }

      if (WithHeads) {
	if (symbols.size() == 2)
	  headpos = 1;
	if (headpos == 0) {
	  char *message=(char*)malloc(1000);
	  sprintf(message, "in line %d of grammar file: rule without head marking", (int)N);
	  throw message;
	}
      }
      rules.push_back(Rule(symbols, headpos));
    }
  }

  if (ss == NULL)
    startsym = 0;
  else {
    int ssn = symbol_number(ss);
    if (ssn < 0) {
      char *message=(char*)malloc(1000);
      sprintf(message, "unknown start symbol \"%s\"!", ss);
      throw message;
    }
    startsym = (SymNum)ssn;
  }

  if (!Quiet)
    cerr << "finished\n";
}


/*******************************************************************/
/*                                                                 */
/*  Grammar::store                                                 */
/*                                                                 */
/*******************************************************************/

void Grammar::store( FILE *file )

{
  for( size_t i=0; i<rules.size(); i++ ) {
    Rule &r=rules[i];
    fprintf( file, "%.2f\t%s", rulefreq[i], symbol_name(r.symbol(0)));
    for( int k=1; k<r.length(); k++ ) {
      if (k == r.headpos)
	fputc( '^', file );
      fprintf( file, " %s", symbol_name(r.symbol(k)));
    }
    fputc('\n', file);
  }
}


/*******************************************************************/
/*                                                                 */
/*  NFGrammar::create_bit_maps                                     */
/*                                                                 */
/*******************************************************************/

void NFGrammar::create_bit_maps( vector<MRule> &chainrule )

{
  // compute bit vector length
  bitvec_length = bitarray_size( g.number_of_symbols() );

  chain.resize( number_of_symbols() );

  // store the chain rules in the normal form grammar
  for( size_t i=0; i<chainrule.size(); i++ ) {
    MRule &rule = chainrule[i];
    SymNum parent = rule.symbol(0);
    SymNum daughter = rule.symbol(1);
    RuleNumber sr = rule.source_rule;
    chain[parent].down.push_back( ChainRule(sr, daughter) );
    chain[daughter].up.push_back( ChainRule(sr, parent) );
  }

  for( size_t i=0; i<number_of_symbols(); i++ ) {
    chain[i].bitvec = new unsigned long[bitvec_length];

    // initialisation
    for( size_t k=0; k<bitvec_length; k++ )
      chain[i].bitvec[k] = 0;

    if (i < g.number_of_symbols())
      set_bit(chain[i].bitvec, i);
  }

  // propagation
  bool repeat;
  do {
    repeat = false;
    for( size_t i=0; i<number_of_symbols(); i++ ) {
      unsigned long *dbv = chain[i].bitvec;
      for( size_t k=0; k<chain[i].up.size(); k++ ) {
	SymNum parent = chain[i].up[k].symbol;
	unsigned long *pbv = chain[parent].bitvec;
	for( size_t l=0; l<bitvec_length; l++ ) {
	  unsigned long old = dbv[l];
	  dbv[l] |= pbv[l];
	  if (old != dbv[l])
	    repeat = true;
	}
      }
    }
  } while (repeat);

  // delete empty bit-vectors 
  vector<size_t> m;
  for( size_t i=0; i<number_of_symbols(); i++ ) {
    extract_bits( 0, (int)g.number_of_symbols(), chain[i].bitvec, m );
    if (m.size() == 0 || 
	(m.size() == 1 && i >= g.number_of_symbols()))
      {
	free(chain[i].bitvec);
	chain[i].bitvec = NULL;
      }
  }
}


/*******************************************************************/
/*                                                                 */
/*  NFGrammar::create_nfg                                          */
/*                                                                 */
/*  Purpose: binarization of the grammar                           */
/*                                                                 */
/*******************************************************************/

void NFGrammar::create_nfg()

{
  SymPairFreq SPF;  // how often occurs each symbol pair?
  SymPairNum SPN;
  vector<MRule> tmprule;
  vector<MRule> chainrule;

  rules_for_sym.resize( g.number_of_symbols() );

  // copy the regular symbol names to the "name" table
  // names of auxiliary symbols will be added later
  for( size_t i=0; i<g.number_of_symbols(); i++ )
    name.push_back(strdup(g.symbol_name((int)i)));

  // insert the grammar rules into tmprule and
  // count the symbol pairs which comprise a rule head
  for( size_t i=0; i<g.rules.size(); i++ ) {
    Rule &r = g.rules[i];
    if (r.length() == 2)
      // unary-branching rule
      chainrule.push_back(MRule(r, (RuleNumber)i));
    else {
      tmprule.push_back(MRule(r, (RuleNumber)i));
      if (WithHeads) {
	unsigned int hp = r.headpos;
	// count the symbol pairs
	if ((int)hp < r.length()-1)
	  SPF[SymPair(r.symbol(hp), r.symbol(hp+1), left)]++;
	if (hp > 1)
	  SPF[SymPair(r.symbol(hp-1), r.symbol(hp), right)]++;
      }
      else
	for( int i=2; i<r.length(); i++ )
	  SPF[SymPair(r.symbol(i-1), r.symbol(i))]++;
    }
  }

  // replace symbol pairs in rules until no rules are left for transformation
  while (tmprule.size() > 0) {
    vector<MRule> newrule;

    for( size_t i=0; i<tmprule.size(); i++ ) {
      MRule &rule = tmprule[i];

      if (rule.length() == 2) {
	// unary rule
	chainrule.push_back(rule);
	continue;
      }

      SymPair p;
      int spos=0, newhp=0;
      if (WithHeads) {
	size_t f = 0;
	int hp = (int)rule.headpos;
	if (hp < rule.length()-1) {
	  p  = SymPair(SymPair(rule.symbol(hp), rule.symbol(hp+1), left));
	  f  = SPF[p];
	  spos = hp;
	}
	if (hp > 1) {
	  SymPair p2(SymPair(rule.symbol(hp-1), rule.symbol(hp), right));
	  if (f < SPF[p2]) {
	    p  = p2;
	    spos = hp-1;
	  }
	}
	newhp = spos;
      }
      else {
	p = SymPair(rule.symbol(1), rule.symbol(2));
	spos = 1;
	for( int i=3; i<rule.length(); i++ ) {
	  SymPair p2(SymPair(rule.symbol(i-1), rule.symbol(i)));
	  if (SPF[p] < SPF[p2]) {
	    p = p2;
	    spos = i-1;
	  }
	}
      }
      if (SPF[p] == 1 && rule.length() == 3) {
	// binary rule whose symbol pair occurs only once
	NFRule nr(rule.symbol(1), rule.symbol(2), p.headdir,rule.source_rule);
	rules_for_sym[rule.symbol(0)].push_back(nr);
	continue;
      }

      // retrieve the auxiliary symbol
      SymNum ns;
      SymPairNum::iterator it=SPN.find(p);
      if (it != SPN.end()) 
	ns = it->second; // symbol pair occurred before
      else {
	// create a new auxiliary symbol
	ns = (SymNum)name.size();
	SPN.insert(SymPairNum::value_type(p, ns));
	
	// store the name of the new auxiliary symbol
	char buffer[1000];
	sprintf(buffer,"%s_%s", name[p.first], name[p.second]);
	name.push_back(strdup(buffer));

	// store the new auxiliary rule
	rules_for_sym.resize( name.size() );
	NFRule nr( p.first, p.second, p.headdir );
	rules_for_sym[ns].push_back(nr);
      }

      vector<int> symbol;
	
      // copy the symbol to the left of the symbol pair
      for( int i=0; i<spos; i++ )
	symbol.push_back(rule.symbol(i));
      
      // add the auxiliary symbol
      symbol.push_back(ns);
	
      // copy the symbols to the right of the symbol pair
      for( int i=spos+2; i<rule.length(); i++ )
	symbol.push_back(rule.symbol(i));

      // add the new rule
      newrule.push_back( MRule(symbol, rule.source_rule, (unsigned short)newhp) );
      // update the counts
      if (WithHeads) {
	if (spos > 1) {
	  SPF[SymPair(symbol[spos-1], symbol[spos], right)]++;
	  if (p.headdir == left)
	    SPF[SymPair(symbol[spos-1], p.first, right)]--;
	}
	if (spos < (int)symbol.size()-1) {
	  SPF[SymPair(symbol[spos], symbol[spos+1], left)]++;
	  if (p.headdir == left)
	    SPF[SymPair(p.second, symbol[spos+1], left)]--;
	}
      }
      else {
	if (spos > 1) {
	  SPF[SymPair(symbol[spos-1], symbol[spos])]++;
	  SPF[SymPair(symbol[spos-1], p.first)]--;
	}
	if (spos < (int)symbol.size()-1) {
	  SPF[SymPair(symbol[spos], symbol[spos+1])]++;
	  SPF[SymPair(p.second, symbol[spos+1])]--;
	}
      }
    }

    tmprule.swap(newrule);
  }

  create_bit_maps( chainrule );
}



/*******************************************************************/
/*                                                                 */
/*  Grammar::estimate_probs                                        */
/*                                                                 */
/*******************************************************************/

void Grammar::estimate_probs( vector<double> &tagfreq, vector<double> &termprob)

{
  vector<double> freqsum;
  freqsum.resize( number_of_symbols(), 0.0 );

  // lexicon
  for( size_t i=0; i<tagfreq.size(); i++ )
    freqsum[i] = tagfreq[i];
  // grammar
  for( size_t i=0; i<rules.size(); i++ )
    freqsum[rules[i].symbol(0)] += rulefreq[i];

  // lexicon
  termprob.resize( tagfreq.size() );
  for( size_t i=0; i<tagfreq.size(); i++ )
    termprob[i] = tagfreq[i] / freqsum[i];
  // grammar
  ruleprob.resize( rulefreq.size() );
  for( size_t i=0; i<rules.size(); i++ ) {
    ruleprob[i] = rulefreq[i] / freqsum[rules[i].symbol(0)];
    rulefreq[i] = 0.0;
  }
}
