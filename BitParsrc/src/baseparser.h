
/*MA****************************************************************/
/*                                                                 */
/*     File: baseparser.h                                          */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Mon Dec 23 09:37:13 2002                              */
/* Modified: Thu Oct 25 16:38:51 2012 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#ifndef _BASEPARSER_H
#define _BASEPARSER_H

#include "Grammar.h"
#include "Lexicon.h"


typedef unsigned int Index;
static const Index MaxIndex=(Index)UINT_MAX;

typedef enum { term, unary, binary, none } RuleType;


/*****************  class NodeIndexTab  ****************************/

class NodeIndexTab {

 private:
  typedef struct {
    SymNum cat;
    unsigned short b,e;
  } Data;

  struct eqdat {
    bool operator()(const Data d1, const Data d2) const {
      return (d1.cat == d2.cat && d1.b == d2.b && d1.e == d2.e);
    }
  };

  struct hashdat {
    size_t operator()(const Data d) const {
      return (d.cat << 16 ^ d.b << 8 ^ d.e);
    }
  };

  typedef hash_map<Data, int, hashdat, eqdat> NodeMap;

  NodeMap NM;
  Index next_index;

 public:

  NodeIndexTab() { next_index = 0; };

  void clear() { NM.clear(); next_index = 0; };

  bool find( SymNum cat, size_t beg, size_t end, Index &n ) {
    Data d = {cat, (unsigned short)beg, (unsigned short)end};
    NodeMap::iterator it = NM.find(d);
    if (it == NM.end())
      return false;
    n = it->second;
    return true;
  };

  void insert( SymNum cat, size_t beg, size_t end, Index n ) {
    Data d = {cat, (unsigned short)beg, (unsigned short)end};
    NM[d] = n;
    if (next_index <= n)
      next_index = n+1;
  };

  Index operator()( SymNum cat, size_t beg, size_t end, bool &new_node ) {
    Data d = {cat, (unsigned short)beg, (unsigned short)end};
    NodeMap::iterator it = NM.find(d);
    if (it == NM.end()) {
      new_node = true;
      return NM.insert(NodeMap::value_type(d, next_index++)).first->second;
    }
    else {
      new_node = false;
      return it->second;
    }
  }

  Index operator()( SymNum cat, size_t beg, size_t end ) {
    bool result;
    return operator()( cat, beg, end, result );
  }

  size_t size() { return next_index; };
};


/*****************  class Analysis  ********************************/

class Analysis {

 public:
  RuleType type;
  unsigned int index;
  unsigned int splitpos;

  Analysis() { type = none; }
  Analysis( RuleType t, unsigned int i ) {
    assert(t == term || t == unary);
    type = t;
    index = i;
  }
  Analysis( RuleType t, unsigned int ri, unsigned int s ) {
    assert(t == binary);
    type = t;
    index = ri;
    splitpos = s;
  }
};


/*****************  class Chart  ***********************************/

class Chart {

 public:
  size_t now;  // size of the sentence
  size_t nos;  // number of non-terminals of the normalised grammar
  size_t number_of_bits;  // maximal bit position (needed for range checks)
  size_t subtabsize;      // number of bits in the subtable for 1 symbol

  // the chart is redundantly stored in two bit vectors
  // with array-like access operators
  unsigned long *C1;  // chart with end position as last index
  unsigned long *C2;  // chart with start position as last index

  // auxiliary bit arrays for speeding up base_parse_op
  unsigned long *F1;  // non-empty flag for each column in first chart
  unsigned long *F2;  // non-empty flag for each column in second chart

  size_t get_pos( SymNum s, size_t b, size_t e ) {
    // translate array indices to bit vector positions in C2
    // use shifting instead of division by 2
    size_t pos = (size_t)s * subtabsize + (size_t)(((e * (e+1)) >> 1) + b);
    return pos;
  };

  size_t get_pos1( SymNum s, size_t b, size_t e ) {
    // translate array indices to bit vector positions in C1
    return get_pos( s, e-b, now-b-1 );
  };

  Chart() { C1 = C2 = F1 = F2 = NULL; };

  void clear() { 
    if (C1 != NULL) { delete[] C1; C1 = NULL;}
    if (C2 != NULL) { delete[] C2; C2 = NULL;}
    if (F1 != NULL) { delete[] F1; F1 = NULL;}
    if (F2 != NULL) { delete[] F2; F2 = NULL;}
  };

  ~Chart() { clear(); };

  void swap( Chart &c ) {
    size_t tmp;
    unsigned long *ctmp;
    tmp = nos; nos = c.nos; c.nos = tmp;
    tmp = now; now = c.now; c.now = tmp;
    ctmp = C1; C1 = c.C1; c.C1 = ctmp;
    ctmp = C2; C2 = c.C2; c.C2 = ctmp;
    ctmp = F1; F1 = c.F1; c.F1 = ctmp;
    ctmp = F2; F2 = c.F2; c.F2 = ctmp;
  };

  void init( size_t n, size_t sc ) {
    // allocate and initialize the data structures
    clear();
    now = n;
    nos = sc;
    subtabsize = (size_t)(now * (now+1) / 2);
    // number of "long"s required:
    number_of_bits = (size_t)nos * subtabsize;
    size_t s = bitarray_size(number_of_bits);
    C1 = new unsigned long[s];
    C2 = new unsigned long[s];
    for( size_t i=0; i<s; i++ )
      C1[i] = C2[i] = 0;
    // auxiliary flags
    s = bitarray_size(now * nos);
    F1 = new unsigned long[s];
    F2 = new unsigned long[s];
    for( size_t i=0; i<s; i++ )
      F1[i] = F2[i] = 0;
  };

  bool get( SymNum s, size_t b, size_t e ) {
    return get_bit(C2, get_pos(s, b, e));
  };

  void set( SymNum sn, size_t b, size_t e);

  inline void set_filter_bits( SymNum sn, size_t b, size_t e ) {
    size_t pos = b * nos + sn;
    size_t x = pos / ULONG_BITS;
    size_t y = pos % ULONG_BITS;
    F1[x] |= (unsigned long)1 << y;
    
    pos = e * nos + sn;
    x = pos / ULONG_BITS;
    y = pos % ULONG_BITS;
    F2[x] |= (unsigned long)1 << y;
  };

  inline bool check_filter_bits( SymNum sn1, SymNum sn2, size_t b, size_t e ) {
    // Check whether a constituent of category sn1 
    // with start position b exists
    size_t pos = b * nos + sn1;
    size_t x = pos / ULONG_BITS;
    size_t y = pos % ULONG_BITS;
    
    if ((F1[x] & ((unsigned long)1 << y))) {
      // Check whether a constituent of category sn2
      // with end position e exists
      pos = e * nos + sn2;
      x = pos / ULONG_BITS;
      y = pos % ULONG_BITS;
      if ((F2[x] & ((unsigned long)1 << y)))
	return true;
    }
    return false;
  };


  bool base_parse_op3(SymNum ls, SymNum rs, size_t b, size_t e, 
		      unsigned long *vec);

  inline bool base_parse_op2(SymNum ls, SymNum rs, size_t b, size_t e, 
			     unsigned long *vec)
    {
      if (!check_filter_bits( ls, rs, b, e ))
	return false;
      
      return base_parse_op3( ls, rs, b, e, vec );
    };

  inline bool base_parse_op(SymNum ls, SymNum rs, size_t b, size_t e, 
			    unsigned long *bitvec, vector<size_t> &m)
    {
      m.clear();

      if (!base_parse_op2(ls, rs, b, e, bitvec))
	return false;

      extract_bits( b, e, bitvec, m );
      
      return true;
    };

  void add_bits( size_t b, size_t e, unsigned long *vec, size_t l );
};


/*****************  class BaseParser  ******************************/

class BaseParser {

private:
  void tags_into_chart();  // add POS tags of words to the chart and apply chain rules
  bool derivable( SymNum s, size_t b, size_t e );
  void compute_chart();
  void filter_node( SymNum cat, size_t b, size_t e, Chart &c );
  Entry *filter_tags( vector<char*> &inputtags, Entry *t );

public:
  Grammar grammar;
  Lexicon lexicon;
  NFGrammar nfg;
  vector<char*> word;
  vector<Entry> tags;
  Chart chart;
  NodeIndexTab nodenumber;
  vector<unsigned short> parentcount;
  vector<Prob> nodeprob;
  vector<Prob> nodefreq;
  vector<double> nodefreqsum;
  bool finished;
  bool SentProbs;
  bool inside_probs;
  
  BaseParser( FILE *gfile, FILE *lfile, char *ss, FILE *ocf, FILE *ocf2, 
	      FILE *wcf, char tss, char tes, double sw, double mlp, double gt, 
	      int msl, bool wl, bool pst )
  : grammar( gfile, ss, tss, tes ), 
    lexicon( lfile, ocf, ocf2, wcf, grammar.symbols, sw, mlp, gt, msl, wl ), 
    nfg( grammar )
    { 
      finished = false; 
      inside_probs = false; 
      grammar.estimate_probs( lexicon.baselexicon.TagFreq, 
			      lexicon.baselexicon.TermProb );
      lexicon.multiply_prior_and_term_prob();
      if (pst) {
	FILE *file = fopen("suffix-trees.txt","wt");
	lexicon.guesser.print( grammar.symbols, file );
	fclose(file);
      }
    };
  
  Index start_node_number() {
    return nodenumber(grammar.start_symbol(), 0, (int)word.size()-1 );
  };

  bool is_auxiliary_symbol( SymNum cat )
    { return cat >= grammar.number_of_symbols(); };

  bool next_chart( FILE* );
  bool next_filtered_chart( FILE* );

  void compute_analyses( SymNum cat, size_t b, size_t e, vector<Analysis>& );
  void insert_tags();
  void apply_chain_rules( SymNum s, size_t b, size_t e, Prob p );

  Prob compute_node_prob( SymNum cat, size_t b, size_t e, vector<char>& );
  void compute_node_freq( SymNum cat, size_t b, size_t e, Index n, vector<char>& );
  double compute_node_freqsums( SymNum cat, size_t b, size_t e, vector<char>& );

  bool next_chart_with_probs( FILE* );
  bool next_chart_with_freqs( FILE *file );
  bool next_chart_with_freqsums( FILE *file );

  void failure_output( FILE* );
  void print_chart( FILE* );

  bool verbose;

  void clear() {
    chart.clear();
    for( size_t i=0; i<word.size(); i++ )
      free(word[i]);
    word.clear();
    tags.clear();
    nodenumber.clear();
    parentcount.clear();
    nodeprob.clear();
    nodefreq.clear();
    nodefreqsum.clear();
  };
};

#endif
