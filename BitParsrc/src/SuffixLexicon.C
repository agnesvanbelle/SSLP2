/*******************************************************************/
/*                                                                 */
/*     File: SuffixLexicon.C                                       */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Sep 13 14:19:57 2007                              */
/* Modified: Fri Jul  6 16:17:22 2012 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/

#include <assert.h>
#include <math.h>

#include "SuffixLexicon.h"

static const int MinSuffFreq = 5;

static const double MinWBFreq = 0.5;


/*******************************************************************/
/*                                                                 */
/*  compute_restprobs                                              */
/*                                                                 */
/*******************************************************************/

void compute_restprobs( SEntry &entry )

{
  double sum = 0.0;
  for( size_t k=0; k<entry.tag.size(); k++ )
    sum += entry.restfreq[k];
  for( size_t k=0; k<entry.tag.size(); k++ )
    entry.restprob[k] = entry.restfreq[k] / (float)sum;
  entry.restfreqsum = (float)sum;
}


/*******************************************************************/
/*                                                                 */
/*  SNode::prepare                                                 */
/*                                                                 */
/*******************************************************************/

void SNode::prepare()

{
  entry.sort_tags();
  entry.restfreq.resize(entry.tag.size());
  entry.restprob.resize(entry.tag.size());

  for( size_t i=0; i<entry.tag.size(); i++ )
    entry.restfreq[i] = entry.tag[i].freq;

  compute_restprobs( entry );
  entry.freq = entry.restfreqsum;

  for( size_t i=0; i<link.size(); i++ )
    link[i].node.prepare();
}


/*******************************************************************/
/*                                                                 */
/*  SNode::store                                                   */
/*                                                                 */
/*******************************************************************/

void SNode::store( FILE *file ) const

{
  entry.store( file );
  size_t n=0;
  for( size_t i=0; i<link.size(); i++ )
    if (link[i].active)
      n++;
  write_data( n, file );
  if (ferror(file))
    errx(1, "Error encountered while writing to file");
  for( size_t i=0; i<link.size(); i++ )
    if (link[i].active)
      link[i].store( file );
}


/*******************************************************************/
/*                                                                 */
/*  SNode::add                                                     */
/*                                                                 */
/*******************************************************************/

void SNode::add( vector<bool> &is_oc_tag, vector<bool> &is_oc_tag_uc, 
		 const Entry &e, const char *first, const char *last, 
		 bool is_upper )
{
  // sum up the tag probabilities of all words with the current suffix
  for( size_t i=0; i<e.tag.size(); i++ ) {
    const Tag &t = e.tag[i];
    if (t.freq > 0 &&
	((is_upper && is_oc_tag_uc[t.number]) ||
	 (!is_upper && is_oc_tag[t.number])))
      entry.add_tag(t.number).freq += t.prob;
  }
  
  // extend the suffix tree
  if (first <= last) {
    size_t i;
    for( i=0; i<link.size(); i++ )
      if (link[i].symbol == *last)
	break;
    if (i == link.size())
      link.push_back(SLink(*last));
    link[i].node.add( is_oc_tag, is_oc_tag_uc, e, first, last-1, is_upper );
  }
}


/*******************************************************************/
/*                                                                 */
/*  compute_info_gain                                              */
/*                                                                 */
/*******************************************************************/

static double compute_info_gain( SEntry &entry, SEntry &parent_entry )

{
  double gain=0.0;
  int tag_count = 0;
  size_t k=0;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    if (entry.restfreq[i] > 0.0) {
      while (entry.tag[i].number != parent_entry.tag[k].number)
	k++;
      gain += entry.restfreq[i] * 
	log(entry.restprob[i] / parent_entry.restprob[k]);
      if (entry.restfreq[i] > MinWBFreq)
	tag_count++;
    }
  }

  assert(gain > -0.00001);
  if (tag_count == 0)
    return 0.0;
  else
    return gain / tag_count;
}


/*******************************************************************/
/*                                                                 */
/*  SNode::subtract_frequencies                                    */
/*                                                                 */
/*******************************************************************/

void SNode::subtract_frequencies( SNode &daughter )

{
  vector<Tag> &mt=entry.tag;
  vector<Tag> &dt=daughter.entry.tag;

  size_t k=0;
  for( size_t i=0; i<dt.size(); i++ ) {
    while (mt[k].number < dt[i].number) 
      k++;
    entry.restfreq[k] -= daughter.entry.restfreq[i];
    // allow rounding errors
    if (entry.restfreq[k] < 0.00001)
      entry.restfreq[k] = 0.0;
  }

  compute_restprobs( entry );
}


/*******************************************************************/
/*                                                                 */
/*  complement_entry                                               */
/*                                                                 */
/*******************************************************************/

void complement_entry( SEntry &result, SEntry &entry, SEntry &parent_entry )

{
  size_t k=0;
  for( size_t i=0; i<parent_entry.tag.size(); i++ ) {
    double f = parent_entry.restfreq[i];
    if (k < entry.tag.size() && 
	entry.tag[k].number == parent_entry.tag[i].number)
      f -= entry.restfreq[k++];
    if (f > 0.01) {
      Tag &rt = result.add_tag(parent_entry.tag[i].number);
      rt.freq = (float)f;
    }
  }

  result.restfreq.resize(result.tag.size());
  result.restprob.resize(result.tag.size());
  for( size_t i=0; i<result.tag.size(); i++ )
    result.restfreq[i] = result.tag[i].freq;

  compute_restprobs( result );
}


/*******************************************************************/
/*                                                                 */
/*  SNode::prune                                                   */
/*                                                                 */
/*******************************************************************/

void SNode::prune( double threshold, char *suffix )

{
  // allocate an array for the probability values

  // determine the set of informative links
  for(;;) {
    // compute the next best link
    double max_gain = 0.0;
    size_t best_link = 0;
    for( size_t i=0; i<link.size(); i++ ) {
      SEntry &e = link[i].node.entry;
      if (!link[i].active) {
	if (e.restfreqsum >= MinSuffFreq) {
	  double gain = compute_info_gain( e, entry );
	  if (max_gain < gain) {
	    max_gain = gain;
	    best_link = i;
	  }
	}
	
	if (e.restfreqsum > entry.restfreqsum * 0.5 && 
	    e.restfreqsum < entry.restfreqsum * 0.99)
	  {
	    SEntry ce;
	    complement_entry( ce, e, entry );
	    if (ce.restfreqsum >= MinSuffFreq) {
	      double gain = compute_info_gain( ce, entry );
	      if (max_gain < gain) {
		max_gain = gain;
		best_link = i;
	      }
	    }
	  }
      }
    }

    if (max_gain <= 0.0)
      break;
    double df = link[best_link].node.entry.restfreqsum;
    double mf = entry.restfreqsum;
    // Is the difference significant or
    // does the best daughter account for more than 90% of the frequencies?
    if (max_gain >= threshold || (df > MinSuffFreq * 2 && df > mf * 0.9)) {
      link[best_link].active = true;
      subtract_frequencies( link[best_link].node );
    }
    else
      break;
  }

  for( size_t i=0; i<link.size(); i++ )
    if (link[i].active) {
      *(suffix-1) = link[i].symbol;
      link[i].node.prune( threshold, suffix-1 );
    }
}


/*******************************************************************/
/*                                                                 */
/*  compute_backoff_probs                                          */
/*                                                                 */
/*******************************************************************/

static void compute_backoff_probs( SEntry &entry, vector<double> &p, 
				   vector<double> &prob )
{
  // compute the "number of observed tags"
  size_t observed_count = 0;
  for( size_t i=0; i<entry.tag.size(); i++ )
    if (entry.tag[i].freq > MinWBFreq)
      observed_count++;

  // Add frequencies and weighted back-off probabilities
  p.clear();
  p.resize(prob.size(), 0.0);
  for( size_t t=0; t<prob.size(); t++ )
    p[t] = (double)observed_count * prob[t];
  for( size_t i=0; i<entry.tag.size(); i++ )
    p[entry.tag[i].number] += entry.tag[i].freq;
  double f = 1.0 / ((double)observed_count + entry.freq);
  double sum=0.0;
  for( size_t t=0; t<p.size(); t++ ) {
    p[t] *= f;
    sum += p[t];
  }
  assert(sum > 0.99999 && sum < 1.00001);
}


/*******************************************************************/
/*                                                                 */
/*  SNode::estimate_probs                                          */
/*                                                                 */
/*******************************************************************/

void SNode::estimate_probs( double mlp, vector<double> &prob )

{
  vector<double> bo_prob;
  compute_backoff_probs( entry, bo_prob, prob );

  vector<double> freq(bo_prob.size(), 0.0);

  // copy the frequencies and compute the number of observed tags
  // and the maximal tag frequency
  size_t observed_count = 0;
  float max_freq = 0;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    freq[entry.tag[i].number] = entry.restfreq[i];
    if (entry.restfreq[i] > MinWBFreq)
      observed_count++;
    if (max_freq < entry.restfreq[i])
      max_freq = entry.restfreq[i];
  }

  // Witten-Bell smoothing; Add the back-off probabilities 
  // multiplied by the number of observed events
  // Eliminate low-probability tags
  if (observed_count == 0)
    observed_count = 1;
  double sum = 0.0;
  for( size_t t=0; t<bo_prob.size(); t++ ) {
    freq[t] += (double)observed_count * bo_prob[t];
    if (freq[t] < max_freq * mlp)
      freq[t] = 0.0;
    else
      sum += freq[t];
  }
  
  // compute the new set of possible tags and their probabilities
  entry.tag.clear();
  for( SymNum t=0; t<(SymNum)freq.size(); t++ )
    if (freq[t] > 0.0) {
      entry.tag.push_back(Tag(t));
      entry.tag.back().prob = (float)((double)freq[t] / sum);
    }
  
  for( size_t i=0; i<link.size(); i++ )
    if (link[i].active)
      link[i].node.estimate_probs( mlp, bo_prob );

  entry.freq = 0.0;
}


/*******************************************************************/
/*                                                                 */
/*  SNode::estimate_probs                                          */
/*                                                                 */
/*******************************************************************/

void SNode::estimate_probs( double mlp )

{
  size_t N=0;
  for( size_t i=0; i<entry.tag.size(); i++ )
    if (N < entry.tag[i].number)
      N = entry.tag[i].number;

  vector<double> prob(N+1 , 0.0);
  double p = 1.0 / (double)entry.tag.size();
  double sum = 0.0;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    prob[entry.tag[i].number] = p;
    sum += prob[entry.tag[i].number];
  }

  assert(sum > 0.99999 && p < 1.00001);
  estimate_probs( mlp, prob );
}


/*******************************************************************/
/*                                                                 */
/*  SNode::multiply_prior_and_term_prob                            */
/*                                                                 */
/*******************************************************************/

void SNode::multiply_prior_and_term_prob( vector<double> &priorprob,
					  vector<double> &termprob, double mwp )
{
  entry.freq = 1;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    Tag &tag = entry.tag[i];
    tag.prob = (float)(tag.prob * termprob[tag.number] * mwp /
		       priorprob[tag.number]);
  }

  for( size_t i=0; i<link.size(); i++ )
    link[i].node.multiply_prior_and_term_prob( priorprob, termprob, mwp );
}


/*******************************************************************/
/*                                                                 */
/*  SNode::lookup                                                  */
/*                                                                 */
/*******************************************************************/

Entry &SNode::lookup( const char *word, int pos )

{
  if (pos >= 0)
    for( size_t i=0; i<link.size(); i++ )
      if (link[i].active && link[i].symbol == word[pos])
	return link[i].node.lookup( word, pos-1 );
  return entry;
}


/*******************************************************************/
/*                                                                 */
/*  SNode::print                                                   */
/*                                                                 */
/*******************************************************************/

void SNode::print( SymbolTable &symtab, FILE *file, char *buffer, int pos ) 
const
{
  fprintf(file,"Suffix=\"");
  for( int i=pos-1; i>=0; i-- )
    fputc( buffer[i], file );
  fputs( "\"\n", file );
  entry.print( symtab, file );
  for( size_t i=0; i<link.size(); i++ ) 
    if (link[i].active) {
      buffer[pos] = link[i].symbol;
      link[i].node.print( symtab, file, buffer, pos+1);
    }
}


/*******************************************************************/
/*                                                                 */
/*  SuffixLexicon::add                                             */
/*                                                                 */
/*******************************************************************/

void SuffixLexicon::add( vector<bool> &is_oc_tag, vector<bool> &is_oc_tag_uc, 
			 const Entry &e, const char *word, int msl)
{
  // check whether this word occurred with an open class tag
  size_t i;
  for( i=0; i<e.tag.size(); i++ )
    if (is_oc_tag[e.tag[i].number] && e.tag[i].freq > 0)
      break;
  if (i == e.tag.size())
    return;
  
  int l=(int)strlen(word);
  if (msl > l)
    msl = l;

  bool is_upper = ((word[0] >= 'A' && word[0] <= 'Z') ||
		   (word[0] >= 'À' && word[0] <= 'Þ'));
  root.add( is_oc_tag, is_oc_tag_uc, e, word+l-msl, word+l-1, is_upper );
}
