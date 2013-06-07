
/*MA****************************************************************/
/*                                                                 */
/*     File: baseparser.C                                          */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Mon Dec 23 09:38:32 2002                              */
/* Modified: Thu Oct 25 16:38:57 2012 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include <time.h>

#include <assert.h>

#include <iostream>
using std::cerr;

#include "baseparser.h"

static clock_t start;


/****************** Basic Operations ******************/

/*******************************************************************/
/*                                                                 */
/*  BaseParser::print_chart                                        */
/*                                                                 */
/*******************************************************************/

void BaseParser::print_chart( FILE *file )

{
  for( size_t b=0; b<word.size(); b++ )
    for( size_t e=b; e<word.size(); e++ )
      for( SymNum s=0; s<nfg.number_of_symbols(); s++ )
	if (chart.get(s, b, e)) {
	  if (nodefreq.size() > 0)
	    fprintf(file, "%d-%d %s %g %g \n", (int)b, (int)e, 
		    nfg.symbol_name(s),
		    (double)nodeprob[nodenumber(s, b, e)],
		    (double)nodefreq[nodenumber(s, b, e)]);
	  else if (nodeprob.size() > 0)
	    fprintf(file, "%d-%d %s %g\n", (int)b, (int)e, nfg.symbol_name(s),
		    (double)nodeprob[nodenumber(s, b, e)]);
	  else
	    fprintf(file, "%u-%u %s\n", (unsigned)b, (unsigned)e, 
		    nfg.symbol_name(s));
	}
  fputc('\n', file);
  return;
}


/*******************************************************************/
/*                                                                 */
/*  Chart::set                                                     */
/*                                                                 */
/*******************************************************************/

void Chart::set( SymNum sn, size_t b, size_t e )

{
  set_bit(C1, get_pos1(sn, b, e ));
  set_bit(C2, get_pos(sn, b, e));
  set_filter_bits( sn, b, e );
}


/*******************************************************************/
/*                                                                 */
/*  Chart::add_bits                                                */
/*                                                                 */
/*******************************************************************/

void Chart::add_bits( size_t b, size_t e, unsigned long *bitvec, size_t l )

{
  vector<size_t> m;
  extract_bits(0, l, bitvec, m);
  for( size_t i=0; i<m.size(); i++ )
    set( (SymNum)m[i], b, e );
}


/*******************************************************************/
/*                                                                 */
/*  Chart::base_parse_op3                                          */
/*                                                                 */
/*******************************************************************/

bool Chart::base_parse_op3( SymNum ls, SymNum rs, size_t b, size_t e, 
			    unsigned long *vec )
{
  // compute positions
  size_t pos = get_pos1(ls, b, b);
  size_t x = pos / ULONG_BITS;
  size_t y = pos % ULONG_BITS;
  size_t l = (e - b) / ULONG_BITS;
  size_t ll = (e - b) % ULONG_BITS;
  
  // copy the bits for the left constituent to vec
  vec[l] = 0;
  unsigned long result = 0;
    
  if (y == 0) {
    for( size_t i=0; i<l; i++ ) {
      vec[i] = C1[x+i];
      result |= vec[i];
    }
    if (ll > 0) {
      // make sure that bits beyond the end position in vec are 0
      vec[l] = (C1[x+l] << (ULONG_BITS - ll)) >> (ULONG_BITS - ll);
      result |= vec[l];
    }
  }
  
  else /* y > 0 */ {
    for( size_t i=0; i<l; i++ ) {
      vec[i] = (C1[x+i] >> y) | (C1[x+i+1] << (ULONG_BITS - y));
      result |= vec[i];
    }
    
    if (y + ll > (int)ULONG_BITS) {
      // bits remaning to be copied belong  to 2 numbers
      vec[l] |= (C1[x+l] >> y) | 
	(C1[x+l+1] << (2 * ULONG_BITS - y - ll) >> (ULONG_BITS - ll));
    }
    else if (ll > 0)
      vec[l] |= (C1[x+l] << (ULONG_BITS - y - ll)) >> (ULONG_BITS - ll);
    result |= vec[l];
  }
  
  if (result) {
    // "and" the bits for the right constituent to vec
    result = 0;
    pos = get_pos(rs, b+1, e);
    x = pos / ULONG_BITS;
    y = pos % ULONG_BITS;
    
    if (y == 0) {
      for( size_t i=0; i<l; i++ ) {
	vec[i] &= C2[x+i];
	result |= vec[i];
      }
      if (ll > 0) {
	vec[l] &= C2[x+l];
	result |= vec[l];
      }
    }
    
    else {
      for( size_t i=0; i<l; i++ ) {
	vec[i] &= (C2[x+i] >> y) | (C2[x+i+1] << (ULONG_BITS - y));
	result |= vec[i];
      }
      
      if (y + ll > ULONG_BITS)
	vec[l] &= (C2[x+l] >> y) | (C2[x+l+1] << (ULONG_BITS - y));
      else if (ll > 0)
	vec[l] &= (C2[x+l] >> y);
      result |= vec[l];
    }
  }
  
  return result != 0;
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::failure_output                                     */
/*                                                                 */
/*******************************************************************/

void BaseParser::failure_output( FILE *file )

{
  fprintf(file, "No parse for: \"%s", word[0]);
  for( size_t i=1; i<word.size(); i++ )
    fprintf(file, " %s", word[i]);
  fputs("\"\n", file);
}


/************** First Pass (Chart is filled bottom up) *************/


/*******************************************************************/
/*                                                                 */
/*  BaseParser::tags_into_chart                                    */
/*                                                                 */
/*******************************************************************/

void BaseParser::tags_into_chart()

{
  unsigned long bitvec[nfg.bitvec_length];
  for( size_t i=0; i<word.size(); i++ ) {
    for( size_t n=0; n<nfg.bitvec_length; n++ )
      bitvec[n] = 0;
    for( size_t k=0; k<tags[i].tag.size(); k++ ) {
      int s = tags[i].tag[k].number;
      for( size_t n=0; n<nfg.bitvec_length; n++ )
	bitvec[n] |= nfg.chain[s].bitvec[n];
    }
    chart.add_bits(i, i, bitvec, grammar.number_of_symbols());
  }
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::derivable                                          */
/*                                                                 */
/*******************************************************************/

bool BaseParser::derivable( SymNum s, size_t b, size_t e )

{
  unsigned long bitvec[(e - b + ULONG_BITS - 1) / ULONG_BITS + 1];
  vector<NFRule> &r=nfg.get_rules(s);
  for( size_t i=0; i<r.size(); i++ ) {
    NFRule &rule = r[i];
    if (chart.base_parse_op2(rule.left, rule.right, b, e, bitvec)) 
      return true;
  }

  return false;
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::compute_chart                                      */
/*                                                                 */
/*******************************************************************/

void BaseParser::compute_chart()

{
  size_t bitvec_length2 = (nfg.number_of_symbols()+ULONG_BITS-1) / ULONG_BITS + 1;
  unsigned long bitvec[bitvec_length2];

  chart.init( word.size(), nfg.number_of_symbols() );
  tags_into_chart();  // Tags from the lexicon are inserted into the chart

  for( int e=1; e<(int)word.size(); e++ ) {
    if (verbose)
      cerr << ".";
    for( int b=e-1; b>=0; b-- ) {
      // initialize the bit vector which stores the categories found
      // for the current start and end position
      for( size_t i=0; i<bitvec_length2; i++ )
	bitvec[i] = 0;
      SymNum s;
      int flag=0;
      for( s=0; s<grammar.number_of_symbols(); s++ ) {
	if (get_bit( bitvec, s ))
	  continue;  // category was previously generated with a chain rule
	if (derivable( s, b, e )) {
	  flag = 1;
	  if (nfg.chain[s].bitvec == NULL)
	    set_bit( bitvec, s );
	  else {
	    unsigned long *v=nfg.chain[s].bitvec;
	    for( size_t i=0; i<nfg.bitvec_length; i++ )
	      bitvec[i] |= v[i];
	  }
	}
      }
      for( ; s<nfg.number_of_symbols(); s++ )
	if (derivable(s, b, e)) {
	  flag = 1;
	  set_bit( bitvec, s );
	  if (nfg.chain[s].bitvec != NULL) {
	    unsigned long *v=nfg.chain[s].bitvec;
	    for( size_t i=0; i<nfg.bitvec_length; i++ )
	      bitvec[i] |= v[i];
	  }
	}
      if (flag)
	chart.add_bits( b, e, bitvec, nfg.number_of_symbols() );
    }
  }
}



/*******************************************************************/
/*                                                                 */
/*  BaseParser::filter_tags                                        */
/*                                                                 */
/*******************************************************************/

Entry *BaseParser::filter_tags( vector<char*> &inputtags, Entry *t )

{
  static Entry result;
  result.freq = t->freq;
  result.tag.clear();

  for( size_t i=0; i<inputtags.size(); i++ ) {
    bool match_found = false;
    for( size_t k=0; k<t->tag.size(); k++ ) {
      const char *tagname = grammar.symbol_name( t->tag[k].number );
      if (strncmp(tagname, inputtags[i], strlen(inputtags[i])) == 0) {
	match_found = true;
	result.tag.push_back(t->tag[k]);
      }
    }
    if (!match_found) {
      // insert any matching POS tags
      vector<SymNum> &tagnum = lexicon.baselexicon.terminal_symbol;
      for( size_t k=0; k<tagnum.size(); k++ ) {
	const char *tagname = grammar.symbol_name( tagnum[k] );
	if (strncmp(tagname, inputtags[i], strlen(inputtags[i])) == 0) {
	  result.tag.push_back(Tag(tagnum[k]));
	  result.tag.back().prob = (float)(1.0 / (double)t->tag.size());
	}
      }
    }
  }
  
  if (result.tag.size() == 0)
    result = *t;
  else {
    // renormalize probabilities
    float sum=0.0;
    for( size_t i=0; i<result.tag.size(); i++ )
      sum += result.tag[i].prob;
    for( size_t i=0; i<result.tag.size(); i++ )
      result.tag[i].prob = result.tag[i].prob / sum;
  }
  
  return &result;
}



/*******************************************************************/
/*                                                                 */
/*  BaseParser::next_chart                                         */
/*                                                                 */
/*******************************************************************/

bool BaseParser::next_chart( FILE *file )

{
  if (verbose) {
    start = clock();
    cerr << "\ncomputing chart";
  }
  clear();

  char buffer[1000];
  bool sstart=true;
  while (fgets(buffer,1000,file)) {
    char *w = strtok(buffer,"\t\n\r");
    vector<char*> inputtags;
    if (w == NULL) {
      if (word.size() == 0)
	continue;
      else
	break; // complete sentence read
    }

    word.push_back(strdup(w));
    Entry *entry = lexicon.lookup(w, sstart);
    char *tagstring;
    while ((tagstring = strtok(NULL," \t\n\r")) != NULL)
      inputtags.push_back(tagstring);
    if (inputtags.size() > 0)
      entry = filter_tags( inputtags, entry );

    if ((w[0] >= 'a' && w[0] <= 'z') || (w[0] >= 'à' && w[0] <= 'ÿ'))
      sstart = false;
    else if (w[0] == '"' || w[0] == '\'' || w[0] == '`' || w[0] == ':')
      sstart = true;
    if (entry == NULL) {
      char *message=(char*)malloc(1000);
      sprintf(message, "unknown word \"%s\"\n", word.back());
      assert(0);
      throw message;
    }
    tags.push_back( *entry );
  }
  if (word.size() == 0) {
    finished = true;
    return false;
  }

  static int N=0;
  if (!Quiet && !verbose)
    cerr << "\r" << ++N;

  compute_chart();

  if (verbose) {
    cerr << "finished\n";
    fprintf( stderr, "time %.3f\n", 
	     (double) (clock() - start) / CLOCKS_PER_SEC);
  }

  if (chart.get(grammar.start_symbol(), 0, word.size()-1))
    return true;

  return false;
}


/************ Second Pass (Chart is filtered top down) *************/

/*******************************************************************/
/*                                                                 */
/*  BaseParser::compute_analyses                                   */
/*                                                                 */
/*******************************************************************/

void BaseParser::compute_analyses( SymNum cat, size_t b, size_t e, 
				   vector<Analysis> &analyses )
{
  // check whether current node is a terminal node
  if (b == e)
    for( unsigned i=0; i<tags[b].tag.size(); i++ )
      if (cat == (SymNum)tags[b].tag[i].number)
	analyses.push_back(Analysis(term, i));

  // look for binary rules
  {
    unsigned long bitvec[(e - b + ULONG_BITS - 1) / ULONG_BITS];
    vector<size_t> m;
    vector<NFRule> &r=nfg.get_rules(cat);
    for( unsigned i=0; i<r.size(); i++ ) {
      NFRule &rule = r[i];
      if (chart.base_parse_op(rule.left, rule.right, b, e, bitvec, m))
	for( size_t k=0; k<m.size(); k++ )
	  analyses.push_back(Analysis(binary, i, (int)m[k]));
    }
  }

  // look for chain rules
  if (!is_auxiliary_symbol(cat)) {
    vector<ChainRule> &cr=nfg.chain[cat].down;
    for( unsigned k=0; k<cr.size(); k++ ) {
      ChainRule &r=cr[k];
      if (chart.get(r.symbol, b, e))
	analyses.push_back(Analysis(unary, k));
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::compute_node_prob                                  */
/*                                                                 */
/*******************************************************************/

Prob BaseParser::compute_node_prob( SymNum cat, size_t b, size_t e, 
				    vector<char> &v )
{
  bool new_node;
  Index n = nodenumber( cat, b, e, new_node );

  if (!new_node) {
    if (inside_probs) {
      if (v[n])
	return (Prob)0.0;
      parentcount[n]++;
    }
    return nodeprob[n];
  }

  // check the size of nodeprob
  size_t s = nodeprob.size();
  if (s <= n) {
    if (s == 0)
      s = 256;
    else
      s *= 2;
    nodeprob.resize(s);
    if (inside_probs) {
      parentcount.resize(s, 0);
      v.resize(s, 0);
    }
  }
  if (inside_probs) {
    v[n] = 1;
    parentcount[n]++;
  }

  vector<Analysis> analysis;
  compute_analyses( cat, b, e, analysis );

  for( size_t i=0; i<analysis.size(); i++ ) {
    if (analysis[i].type == term) {
      Prob p = (Prob)tags[b].tag[analysis[i].index].prob;
      if (p == (Prob)0.0)
	throw "Error in function compute_node_prob: lexicon entry with zero probability!\n";
      if (inside_probs)
	nodeprob[n] += p;
      else if (nodeprob[n] < p)
	nodeprob[n] = p;
      assert(nodeprob[n] > (Prob)0.0);
    }
    else if (analysis[i].type == unary) {
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      // compute the inside probability of the analysis
      Prob p = grammar.ruleprob[cr.source_rule];
      p *= compute_node_prob( cr.symbol, b, e, v );

      if (inside_probs)
	nodeprob[n] += p;
      else if (nodeprob[n] < p)
	nodeprob[n] = p;
    }
    else if (analysis[i].type == binary) {
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];

      // compute the inside probability of the analysis
      Prob p = nfg.ruleprob(rule.source_rule); 
      p *= compute_node_prob( rule.left, b, analysis[i].splitpos, v );
      p *= compute_node_prob( rule.right, analysis[i].splitpos+1, e, v );

      if (inside_probs)
	nodeprob[n] += p;
      else if (nodeprob[n] < p)
	nodeprob[n] = p;
    }
  }

  if (inside_probs)
    v[n] = 0;
  return nodeprob[n];
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::next_chart_with_probs                              */
/*                                                                 */
/*******************************************************************/

bool BaseParser::next_chart_with_probs( FILE *file )

{
  if (!next_chart( file )) {
    chart.clear();
    return false;
  }

  if (verbose) {
    start = clock();
    cerr << "computing probabilities...";
  }

  vector<char> v;
  compute_node_prob( grammar.start_symbol(), 0, word.size()-1, v );
  nodeprob.resize(nodenumber.size());

  if (verbose) {
    cerr << "finished\n";
    fprintf( stderr, "time %.3f\n", 
	     (double) (clock() - start) / CLOCKS_PER_SEC);
  }

  return true;
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::compute_node_freq                                  */
/*                                                                 */
/*******************************************************************/

void BaseParser::compute_node_freq( SymNum cat, size_t b, size_t e, Index n, 
				    vector<char> &v )

{
  // Have all parents been processed?
  if (v[n] || --parentcount[n] > 0)
    return;
  v[n] = 1;

  // compute estimated node frequencies by propagating the frequency
  // of a parent node to the daughter nodes of the different analyses
  // in proportion to the inside probability of the respective
  // analysis

  Prob f = nodefreq[n];
  if (f > (Prob)0.0)
    f = f / nodeprob[n]; // frequency divided by the inside probability
  vector<Analysis> analysis;
  compute_analyses( cat, b, e, analysis );

  for( size_t i=0; i<analysis.size(); i++ ) {
    if (analysis[i].type == unary) {
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      Index d = nodenumber( cr.symbol, b, e );

      if (!v[d]) {
	// compute the inside probability of the analysis
	Prob p = grammar.ruleprob[cr.source_rule] * nodeprob[d];

	// compute the estimated frequency of the analysis and
	// add it to the daughter node
	nodefreq[d] += p * f;

	compute_node_freq( cr.symbol, b, e, d, v );
      }
    }
    else if (analysis[i].type == binary) {
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];
      Index ld = nodenumber( rule.left, b, analysis[i].splitpos );
      Index rd = nodenumber(rule.right, analysis[i].splitpos+1, e);

      // compute the inside probability of the analysis
      Prob p = nfg.ruleprob(rule.source_rule) * nodeprob[ld] * nodeprob[rd];

      // compute the estimated frequency of the analysis and
      // add it to the daughter node
      nodefreq[ld] += p * f;
      nodefreq[rd] += p * f;

      compute_node_freq( rule.left, b, analysis[i].splitpos, ld, v );
      compute_node_freq( rule.right, analysis[i].splitpos+1, e, rd, v );
    }
  }
  v[n] = 0;
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::next_chart_with_freqs                              */
/*                                                                 */
/*******************************************************************/

bool BaseParser::next_chart_with_freqs( FILE *file )

{
  inside_probs = true; 
  if (!next_chart_with_probs( file ))
    return false;

  if (verbose) {
    start = clock();
    cerr << "computing estimated frequencies...";
  }

  nodefreq.resize(nodenumber.size());
  vector<char> v(nodenumber.size(), 0);
  Index n = start_node_number();
  nodefreq[n] = 1.0;
  compute_node_freq( grammar.start_symbol(), 0, word.size()-1, n, v );

  if (verbose) {
    cerr << "finished\n";
    fprintf( stderr, "time %.3f\n", 
	     (double) (clock() - start) / CLOCKS_PER_SEC);
  }

  return true;
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::compute_node_freqsums                              */
/*                                                                 */
/*******************************************************************/

double BaseParser::compute_node_freqsums( SymNum cat, size_t b, size_t e, 
					  vector<char> &v)
{
  Index n = nodenumber( cat, b, e );

  if (v[n] == 1)
    return -DBL_MAX; // recursion
  else if (v[n] == 2)
    return nodefreqsum[n];
  v[n] = 1;

  double p=0.0;
  if (!is_auxiliary_symbol(cat)) {
    p = nodefreq[n];
    p -= 1 - p;
  }

  vector<Analysis> analysis;
  compute_analyses( cat, b, e, analysis );

  for( size_t i=0; i<analysis.size(); i++ ) {
    if (analysis[i].type == term) {
      if (nodefreqsum[n] < p)
	nodefreqsum[n] = p;
    }
    else if (analysis[i].type == unary) {
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      double pp = compute_node_freqsums( cr.symbol, b, e, v ) + p;
      if (nodefreqsum[n] < pp)
	nodefreqsum[n] = pp;
    }
    else if (analysis[i].type == binary) {
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];
      double pp = compute_node_freqsums(rule.left, b, analysis[i].splitpos, v)
	+ compute_node_freqsums(rule.right, analysis[i].splitpos+1, e, v) + p;
      if (nodefreqsum[n] < pp)
	nodefreqsum[n] = pp;
    }
  }
  v[n] = 2;
  return nodefreqsum[n];
}


/*******************************************************************/
/*                                                                 */
/*  BaseParser::next_chart_with_freqsums                           */
/*                                                                 */
/*******************************************************************/

bool BaseParser::next_chart_with_freqsums( FILE *file )

{
  if (!next_chart_with_freqs( file )) {
    chart.clear();
    return false;
  }

  if (verbose) {
    start = clock();
    cerr << "computing scores...";
  }

  nodefreqsum.resize(nodenumber.size(),-DBL_MAX);
  vector<char> v(nodenumber.size(), 0);
  compute_node_freqsums( grammar.start_symbol(), 0, word.size()-1, v );

  if (verbose) {
    cerr << "finished\n";
    fprintf( stderr, "time %.3f\n", 
	     (double) (clock() - start) / CLOCKS_PER_SEC);
  }

  return true;
}
