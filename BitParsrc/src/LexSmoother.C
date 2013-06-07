
/*******************************************************************/
/*                                                                 */
/*     File: LexSmoother.C                                         */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Wed Jul 25 17:24:58 2007                              */
/* Modified: Fri Oct 19 14:50:21 2012 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/


#include "LexSmoother.h"


// The tag probabilities of words are smoothed with Witten-Bell
// smoothing and a backoff probability distribution which is the
// average tag probability distribution of the words with the same 
// set of possible tags.


/*******************************************************************/
/*                                                                 */
/*  entry_cmp                                                      */
/*                                                                 */
/*******************************************************************/

int entry_cmp( const Entry &e1, const Entry &e2 )

{
  // compare the sets od possible tags of two words
  int d = (int)e1.tag.size() - (int)e2.tag.size();
  if (d)
    return d;
  for( size_t i=0; i<e1.tag.size(); i++ ) {
    d = (int)e1.tag[i].number - (int)e2.tag[i].number;
    if (d)
      return d;
  }
  return 0;
}


/*******************************************************************/
/*                                                                 */
/*  LexSmoother::find_entry                                        */
/*                                                                 */
/*******************************************************************/

size_t LexSmoother::find_entry( Entry &entry )

{
  // find the respective set of possible tags in the classentry table
  size_t l = 0;
  size_t r = classentry.size();
  while (l < r) {
    size_t m = (l + r) >> 1;
    if (entry_cmp( classentry[m], entry ) < 0)
      l = m+1;
    else 
      r = m;
  }
  return l;
}


/*******************************************************************/
/*                                                                 */
/*  LexSmoother::add_entry                                         */
/*                                                                 */
/*******************************************************************/

void LexSmoother::add_entry( Entry &entry )

{
  // add the set of possible tags to the classentry table and
  // add the probabilities to the sum of probabilities stored in classentry 

  // find the entry with binary search
  size_t n = find_entry( entry );

  // new entry?
  if (n == classentry.size() || entry_cmp(classentry[n], entry) != 0) {
    // create space for the new element
    classentry.resize(classentry.size() + 1);
    for( size_t i=classentry.size()-1; i>n; i-- )
      classentry[i].tag.swap(classentry[i-1].tag);

    // create a new element
    classentry[n].tag.reserve(entry.tag.size());
    for( size_t i=0; i<entry.tag.size(); i++ )
      classentry[n].tag.push_back(Tag(entry.tag[i].number));
  }

  // Sum up the probs
  for( size_t i=0; i<entry.tag.size(); i++ )
    classentry[n].tag[i].prob += entry.tag[i].prob;

  classentry[n].freq += (float)1.0;
}


/*******************************************************************/
/*                                                                 */
/*  LexSmoother::smooth_entry                                      */
/*                                                                 */
/*******************************************************************/

void LexSmoother::smooth_entry( const char *word, Entry &entry, 
				vector<double> &prior_prob, 
				vector<bool> &is_oc_tag  )
{
  // look up the class entry
  Entry &centry = classentry[find_entry( entry )];

  // Witten-Bell smoothing
  // compute the number of observed tags
  size_t observed=0;
  bool has_oc_tag=false;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    Tag &tag = entry.tag[i];
    if (tag.freq > 0.5)
      observed++;
    has_oc_tag |= is_oc_tag[tag.number];
    tag.prob = (float)tag.freq;
  }
  if (observed == 0)
    observed = 1;
  
  // Define the weights of the different backoff functions
  double sw1 = 0.0;
  double sw2 = 0.0;
  double sw3 = 0.0;
  if (has_oc_tag)
    sw2 = 1.0;
  if (observed < entry.tag.size()) {
    if (centry.freq > 1.0)
      sw1 = 1.0;
    else
      sw3 = 1.0;
  }
  double sum = sw1 + sw2 + sw3;
  sw1 = sw1 / sum;
  sw2 = sw2 / sum;
  sw3 = sw3 / sum;
  
  // smooth the frequencies by adding the weighted class-based probabilities
  if (sw1 > 0.0)
    for( size_t i=0; i<entry.tag.size(); i++ )
      entry.tag[i].prob += 
	(float)((double)observed * sw1 * centry.tag[i].prob);
  
  // smooth the frequencies by adding the guesser-based probabilities
  if (sw2 > 0.0) {
    Entry *ge=guesser.lookup(word);
    for( size_t i=0; i<ge->tag.size(); i++ )
      entry.add_tag( ge->tag[i].number ).prob += 
	(float)((double)observed * sw2 * ge->tag[i].prob);
  }

  // smooth the frequencies by adding the prior tag probabilities
  if (sw3 > 0.0)
    for( size_t i=0; i<entry.tag.size(); i++ ) {
      Tag &tag = entry.tag[i];
      tag.prob += (float)((double)observed * sw3 * prior_prob[tag.number]);
    }
  
  sum = 0.0;
  double max_freq = 0.0;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    float f = entry.tag[i].prob;
    if (max_freq < f)
      max_freq = f;
    sum += f;
  }

  // normalize the frequencies to obtain probabilities
  float min_prob = (float)(max_freq / sum * MinLexProb);
  size_t k=0;
  for( size_t i=0; i<entry.tag.size(); i++ ) {
    entry.tag[i].prob = (float)(entry.tag[i].prob / sum);
    if (entry.tag[i].prob >= min_prob)
      entry.tag[k++] = entry.tag[i];
  }
  entry.tag.resize(k);
}


/*******************************************************************/
/*                                                                 */
/*  LexSmoother::LexSmoother                                       */
/*                                                                 */
/*******************************************************************/

LexSmoother::LexSmoother( BaseLexicon &lex, Guesser &g, 
			  double sw, double mlp )
  : SmoothingWeight(sw), MinLexProb(mlp), guesser(g)
{
  // Sum up the tag probabilities for each class
  for( size_t i=0; i<lex.size(); i++ )
    add_entry( lex.get_entry(i) );

  // estimate the class-based tag probabilities
  for( size_t i=0; i<classentry.size(); i++ ) {
    Entry &entry = classentry[i];

    double sum=0.0;
    for( size_t k=0; k<entry.tag.size(); k++ )
      sum += entry.tag[k].prob;

    for( size_t k=0; k<entry.tag.size(); k++ )
      entry.tag[k].prob = (float)(entry.tag[k].prob / sum);
  }
  

  // Smooth the lexicon entries
  for( BaseLexicon::iterator it=lex.begin(); it!=lex.end(); it++ )
    smooth_entry( it->first, lex.get_entry(it->second), lex.PriorProb,
		  guesser.is_oc_tag  );
}
