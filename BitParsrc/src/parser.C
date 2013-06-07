/*******************************************************************/
/*      File: parser.C                                             */
/*    Author: Helmut Schmid                                        */
/*   Purpose:                                                      */
/*   Created: Tue Nov  5 11:55:39 2002                             */
/*  Modified: Thu Oct 25 16:39:06 2012 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include <time.h>

#include <algorithm>
using std::sort;

#include <iostream>
using std::cerr;

#include "parser.h"

#include "print.h"
#include "print-YAP.h"


Parser *Edge::iterator::parser=NULL;
Parser *Node::iterator::parser=NULL;

static clock_t start;


/*******************************************************************/
/*                                                                 */
/*  Parser::prune_analyses                                         */
/*                                                                 */
/*******************************************************************/

void Parser::prune_analyses( SymNum cat, size_t b, size_t e, 
			     vector<Analysis> &analysis )
{
  vector<Analysis> pruned_analysis;
  Index n = nodenumber(cat, b, e);
  Prob best_freq = (Prob)0.0;
  size_t best_analysis = 0;
  Prob fac = nodefreq[n] / nodeprob[n];

  for( size_t i=0; i<analysis.size(); i++ ) {
    Prob f;
    if (analysis[i].type == term)
      f = fac * (Prob)tags[b].tag[analysis[i].index].prob;
    else if (analysis[i].type == unary) {
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      Index d = nodenumber(cr.symbol, b, e);
      f = fac * grammar.ruleprob[cr.source_rule] * nodeprob[d];
    }
    else { // binary rule
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];
      RuleNumber rn = rule.source_rule;
      Index ld = nodenumber(rule.left, b, analysis[i].splitpos);
      Index rd = nodenumber(rule.right, analysis[i].splitpos+1, e);
      f = fac * nfg.ruleprob(rn) * nodeprob[ld] * nodeprob[rd];
    }
      
    if (f > PruningThreshold) 
      pruned_analysis.push_back(analysis[i]);
    else if (pruned_analysis.size() == 0 && best_freq < f) {
      best_freq = f;
      best_analysis = i;
    }
  }

  if (pruned_analysis.size() == 0)
    pruned_analysis.push_back( analysis[best_analysis] );
  analysis.swap(pruned_analysis);
}


/*******************************************************************/
/*                                                                 */
/*  Parser::build_parse                                            */
/*                                                                 */
/*******************************************************************/

Index Parser::build_parse( SymNum cat, size_t b, size_t e, NodeIndexTab &NH )
  
{
  Index node_id;
  if (NH.find( cat, b, e, node_id ))
    return node_id;
  
  vector<Analysis> analysis;
  compute_analyses( cat, b, e, analysis );
  if (PruningThreshold > (Prob)0.0)
    prune_analyses( cat, b, e, analysis );
    
  int dcount=0;
  for( size_t i=0; i<analysis.size(); i++ )
    if (analysis[i].type == unary)
      dcount++;
    else if (analysis[i].type == binary)
      dcount+=2;

  node_id = (Index)parse.symbol.size();
  NH.insert( cat, b, e, node_id );

  size_t l = analysis.size();
  Index apos = (Index)parse.rule_data.size();
  Index dpos = (Index)parse.daughter.size();

  parse.symbol.push_back(cat);
  parse.first_analysis.push_back((Index)apos);

  parse.rule_data.resize(apos + l);
  parse.first_daughter.resize(apos + l);
  parse.daughter.resize(dpos + dcount);

  for( size_t i=0; i<analysis.size(); i++ ) {
    if (analysis[i].type == term) {
      parse.rule_data[apos] = 
	parse.terminal_rule_data( (Index)b, analysis[i].index);
      parse.first_daughter[apos++] = dpos;
    }
    else if (analysis[i].type == unary) {
      parse.rule_data[apos] = parse.unary_rule_data(analysis[i].index);
      parse.first_daughter[apos++] = dpos;
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      Index n = build_parse( cr.symbol, b, e, NH );
      parse.daughter[dpos++] = n;
    }
    else { // binary rule
      parse.rule_data[apos] = parse.binary_rule_data(analysis[i].index);
      parse.first_daughter[apos++] = dpos;
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];
      // Direct assignment of the result of build_parse to
      // parse.daughter doesn't work (probably because the array
      // "daughter" is sometimes resized and moved during the function call
      Index n = build_parse( rule.left, b, analysis[i].splitpos, NH );
      parse.daughter[dpos++] = n;
      n = build_parse( rule.right, analysis[i].splitpos+1, e, NH );
      parse.daughter[dpos++] = n;
    }
  }

  return node_id;
}


/*******************************************************************/
/*                                                                 */
/*  Parser::next_parse                                             */
/*                                                                 */
/*******************************************************************/

Parse *Parser::next_parse( FILE *file )

{
  clear();
  NodeIndexTab NH;

  // compute the next chart
  if (PruningThreshold > (Prob)0.0) {
    if (!next_chart_with_freqs( file ))
      return NULL;
  }
  else if (!next_chart( file )) {
    chart.clear();
    return NULL;
  }

  if (verbose)
    cerr << "\nbuilding parse...";
  build_parse( grammar.start_symbol(), 0, word.size()-1, NH);
  parse.number_of_roots = 1;
  parse.first_analysis.push_back( (Index)parse.rule_data.size() );
  parse.first_daughter.push_back( (Index)parse.daughter.size() );
  if (verbose)
    cerr << "finished\n";

  chart.clear();

  if (Lexicalized) {
    if (verbose) {
      cerr << "\nlexicalization...";
      start = clock();
    }
    lexicalize_parse();
    if (verbose) {
      cerr << "finished\n";
      fprintf( stderr, "time %.3lf\n", 
	       (double) (clock() - start) / CLOCKS_PER_SEC);
      cerr << parse.number_of_nodes() << " nodes after lexicalization\n";
    }
  }

  if (EstimatedFreqs) {
    if (verbose)
      cerr << "frequency estimation...";
    estimate_freqs();
    if (verbose)
      cerr << "finished\n";
    if (Viterbi) {
      if (Verbose)
	cerr << "probability computation...";
      viterbi();
      if (Verbose)
	cerr << "finished\n";
    }
  }
  else if (Training) {
    if (verbose)
      cerr << "probability computation...";
    train();
    if (verbose)
      cerr << "finished\n";
  }
  else if (Viterbi || ViterbiProbs || NBest) {
    if (verbose)
      cerr << "probability computation...";
    viterbi();
    if (verbose)
      cerr << "finished\n";
  }
  else if (InsideProbs) {
    if (verbose)
      cerr << "probability computation...";
    inside();
    if (verbose)
      cerr << "finished\n";
  }

  return &parse;
}
