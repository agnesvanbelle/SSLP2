
/*MA****************************************************************/
/*                                                                 */
/*     File: vitparser.C                                           */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Mon Dec 23 09:41:19 2002                              */
/* Modified: Thu Oct 25 16:39:01 2012 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include <time.h>

#include <assert.h>

#include <iostream>
using std::cerr;

#include "vitparser.h"
#include "quote.h"

static clock_t start;


/*******************************************************************/
/*                                                                 */
/*  VitParser::build_subtree                                       */
/*                                                                 */
/*******************************************************************/

VParse *VitParser::build_subtree( SymNum cat, size_t b, size_t e, Index n )

{
  Prob vp = nodeprob[n] * 0.999999999; // allow for rounding inaccuracies 
                                       // in comparisons

  vector<Analysis> analysis;
  compute_analyses( cat, b, e, analysis );

  for( size_t i=0; i<analysis.size(); i++ ) {
    if (analysis[i].type == term) {
      Prob p = (Prob)tags[b].tag[analysis[i].index].prob;
      if (p > vp) // allow rounding inaccuracy
	return new VParse( cat, word[b] );
    }
    else if (analysis[i].type == unary) {
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      Index d = nodenumber( cr.symbol, b, e );
      Prob p = nodeprob[d] * grammar.ruleprob[cr.source_rule];

      if (p > vp) { // allow rounding inaccuracy
	// The following two lines should not be merged
	// (Is the "new" operator not reentrant?)
	VParse *dp = build_subtree(cr.symbol, b, e, d);
	return new VParse(cat, cr.source_rule, dp);
      }
    }

    else if (analysis[i].type == binary) {
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];
      Index ld = nodenumber( rule.left, b, analysis[i].splitpos );
      Index rd = nodenumber(rule.right, analysis[i].splitpos+1, e);

      // compute the inside probability of the analysis
      Prob p = nfg.ruleprob(rule.source_rule) * nodeprob[ld] * nodeprob[rd];

      if (p > vp) { // allow rounding inaccuracy
	// The following three lines should not be merged
	VParse *lp = build_subtree(rule.left, b, analysis[i].splitpos, ld);
	VParse *rp = build_subtree(rule.right, analysis[i].splitpos+1, e, rd);
	return new VParse( cat, rule.source_rule, lp, rp );
      }
    }
  }

  assert(0);
  throw "program error in function build_subtree!\n";
}


/*******************************************************************/
/*                                                                 */
/*  VitParser::build_subtree2                                      */
/*                                                                 */
/*******************************************************************/

VParse *VitParser::build_subtree2( SymNum cat, size_t b, size_t e, Index n,
				   vector<char> &v )
{
  v[n] = 1;

  double score;
  if (nodefreqsum[n] > 0.0)
    score = nodefreqsum[n] * 0.9999999999;
  else
    score = nodefreqsum[n] * 1.0000000001;
  double p=0.0;
  if (!is_auxiliary_symbol(cat)) {
    p = nodefreq[n];
    p -= 1 - p;
  }

  vector<Analysis> analysis;
  compute_analyses( cat, b, e, analysis );

  VParse *result = NULL;
  for( size_t i=0; i<analysis.size(); i++ ) {
    if (analysis[i].type == term) {
      result = new VParse( cat, word[b] );
      break;
    }
    else if (analysis[i].type == unary) {
      ChainRule &cr=nfg.chain[cat].down[analysis[i].index];
      Index d = nodenumber( cr.symbol, b, e );
      double pscore = nodefreqsum[d] + p;

      if (pscore > score && !v[d]) { // allow rounding inaccuracy
	// The following two lines should not be merged
	// (Is the "new" operator not reentrant?)
	VParse *dp = build_subtree2(cr.symbol, b, e, d, v);
	result = new VParse(cat, cr.source_rule, dp);
	break;
      }
    }

    else if (analysis[i].type == binary) {
      NFRule &rule = nfg.get_rules(cat)[analysis[i].index];
      Index ld = nodenumber( rule.left, b, analysis[i].splitpos );
      Index rd = nodenumber( rule.right, analysis[i].splitpos+1, e);
      double pscore = nodefreqsum[ld] + nodefreqsum[rd] + p;

      if (pscore > score) { // allow rounding inaccuracy
	// The following three lines should not be merged
	VParse *lp=build_subtree2(rule.left, b, analysis[i].splitpos, ld, v);
	VParse *rp=build_subtree2(rule.right, analysis[i].splitpos+1, e,rd,v);
	result = new VParse( cat, rule.source_rule, lp, rp );
	break;
      }
    }
  }

  v[n] = 0;
  assert(result);
  return result;
}


/*******************************************************************/
/*                                                                 */
/*  VitParser::next_parse                                          */
/*                                                                 */
/*******************************************************************/

bool VitParser::next_parse( FILE *file )

{
  clear();

  if (MaxFScore) {
    if (!next_chart_with_freqsums(file))
      return false;
  }
  else if (!next_chart_with_probs(file))
    return false;

  if (verbose) {
    start = clock();
    cerr << "building parse tree...";
  }
  Index n = start_node_number();

  if (nodeprob[n] == (Prob)0.0) {
    assert(0);
    throw "Error in function next_parse: All parses have zero probability!\n";
  }

  VParse *p;
  if (MaxFScore) {
    vector<char> v(nodenumber.size(), 0);
    p = build_subtree2( grammar.start_symbol(), 0, word.size()-1, n, v );
  }
  else
    p = build_subtree( grammar.start_symbol(), 0, word.size()-1, n);
  parse.push_back( p );
  vitprob.push_back(nodeprob[n]);
  if (verbose) {
    cerr << "finished\n";
    fprintf( stderr, "time %.3f\n", 
	     (double) (clock() - start) / CLOCKS_PER_SEC);
  }
  chart.clear();

  return true;
}


/*******************************************************************/
/*                                                                 */
/*  VitParser::print_subtree                                       */
/*                                                                 */
/*******************************************************************/

void VitParser::print_subtree( VParse *parse, int rn, 
			       size_t &tpos, size_t &dpos, FILE *file )
{
  const char *name=grammar.symbol_name(parse->symbol);

  if (name == NULL) {
    // auxiliary node
    print_subtree( parse->data.nterm.left, rn, tpos, dpos, file);
    print_subtree( parse->data.nterm.right, rn, tpos, dpos, file);
    return;
  }

  if (dpos == 0)
    grammar.traces.print_trace( rn, tpos, dpos, file );

  fprintf(file,"(%s ", quote(name));
  if (parse->termp)
    fprintf(file,"%s)", quote(parse->data.term.word));
  else {
    size_t tp=0, dp=0;
    print_subtree( parse->data.nterm.left, 
		   parse->data.nterm.rule_number, tp, dp, file);
    if (parse->data.nterm.right != NULL)
      print_subtree( parse->data.nterm.right, 
		     parse->data.nterm.rule_number, tp, dp, file);
    fputc(')', file);
  }

  grammar.traces.print_trace( rn, tpos, ++dpos, file );
}


/*******************************************************************/
/*                                                                 */
/*  VitParser::print_parse                                         */
/*                                                                 */
/*******************************************************************/

void VitParser::print_parse( FILE *file )

{
  if (parse.size() == 0)
    failure_output( file );
  else {
    size_t tp=0, dp=0;
    for( size_t i=0; i<parse.size(); i++ ) {
      if (PrintProbs)
	fprintf(file, "logvitprob=%lg\n", vitprob[i].log_val());
      print_subtree( parse[i], -1, tp, dp, file );
      fputc('\n', file);
    }
  }
  if (parse.size() > 1)
    fputc('\n', file);
  fflush(file);
}
