
/*******************************************************************/
/*                                                                 */
/*     File: Lexicon.C                                             */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Feb 20 17:49:38 2009                              */
/* Modified: Fri Oct 19 15:03:16 2012 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/

#include <map>
using std::map;


#include "Lexicon.h"

typedef map<SymNum,Tag> TagTab;


/*******************************************************************/
/*                                                                 */
/*  is_upper                                                       */
/*                                                                 */
/*******************************************************************/

static bool is_upper( char c )

{
  return (c >= 'A' && c <= 'Z') || (c >= 'À' && c <= 'Þ');
}


/*******************************************************************/
/*                                                                 */
/*  decap                                                          */
/*                                                                 */
/*******************************************************************/

static const char *decap( const char *word )

{
  static char buffer[1000];
  if (!is_upper(word[0]) || is_upper(word[1]))
    return NULL;
  buffer[0] = (char)(word[0] + 32);
  int i;
  for( i=1; word[i] && i<999; i++ )
    buffer[i] = word[i];
  buffer[i] = 0;
  return (const char*)buffer;
}

/*******************************************************************/
/*                                                                 */
/*  Lexicon::Lexicon                                               */
/*                                                                 */
/*******************************************************************/

Lexicon::Lexicon( FILE *lexfile, FILE *ocf, FILE *ocf2, FILE *wcfile, 
		  SymbolTable &symtab, double sw, double mlp, 
		  double GuesserGain, int msl, bool wl ) :
  symbols( symtab ), baselexicon( lexfile, symbols, wl ), 
  guesser( wcfile, ocf, ocf2, symbols, baselexicon, GuesserGain, msl, mlp )
  
{
  LexSmoother ls( baselexicon, guesser, sw, mlp );
}


/*******************************************************************/
/*                                                                 */
/*  Lexicon::multiply_prior_and_term_prob                          */
/*                                                                 */
/*******************************************************************/

void Lexicon::multiply_prior_and_term_prob()

{
  double min_word_prob = baselexicon.multiply_prior_and_term_prob();
  guesser.multiply_prior_and_term_prob( baselexicon.PriorProb, 
					baselexicon.TermProb,
					min_word_prob * 0.1);
}


/*******************************************************************/
/*                                                                 */
/*  Lexicon::lookup                                                */
/*                                                                 */
/*******************************************************************/

Entry *Lexicon::lookup( const char *word, bool sstart )

{
  Entry *entry = baselexicon.lookup( word );
  Entry *entry2 = NULL;
  const char *lc_word = (sstart)? decap(word): NULL;
  bool known=true;

  if (lc_word != NULL)
    entry2 = baselexicon.lookup(lc_word);

  if (entry == NULL && entry2 == NULL) {
    entry = guesser.lookup( word );
    if (lc_word != NULL)
      entry2 = guesser.lookup(lc_word);
    known = false;
  }

  if (entry == NULL && entry2 != NULL)
    return entry2;
  if (entry != NULL && entry2 == NULL)
    return entry;
  if (entry == NULL && entry2 == NULL)
    return NULL;

  // merge the two entries

  TagTab tagtab;
  for( size_t i=0; i<entry->tag.size(); i++ ) {
    TagTab::iterator it = tagtab.find(entry->tag[i].number);
    if (it == tagtab.end()) {
      SymNum sym = entry->tag[i].number;
      Tag &tag = entry->tag[i];
      it = tagtab.insert(TagTab::value_type(sym, tag)).first;
      it->second.prob = 0.0;
    }
    it->second.prob += (float)(entry->freq+0.1) * entry->tag[i].prob;
  }
  for( size_t i=0; i<entry2->tag.size(); i++ ) {
    TagTab::iterator it = tagtab.find(entry2->tag[i].number);
    if (it == tagtab.end()) {
      SymNum sym = entry2->tag[i].number;
      Tag &tag = entry2->tag[i];
      it = tagtab.insert(TagTab::value_type(sym, tag)).first;
      it->second.prob = 0.0;
    }
    if (known)
      it->second.prob += (float)(entry2->freq+0.1) * entry2->tag[i].prob;
    else
      it->second.prob += (float)(0.1 * (entry2->freq + 0.1) * 
				 entry2->tag[i].prob);
  }

  static Entry result;
  result.freq = entry->freq + entry2->freq;
  result.tag.resize(0);
  double sum = 0.0;
  for( TagTab::iterator it = tagtab.begin(); it!=tagtab.end(); it++ )
    sum += it->second.prob;
  for( TagTab::iterator it = tagtab.begin(); it!=tagtab.end(); it++ ) {
    it->second.prob = (float)(it->second.prob / sum);
    result.tag.push_back( it->second );
  }

  return &result;
}
