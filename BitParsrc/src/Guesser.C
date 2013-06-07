
/*******************************************************************/
/*                                                                 */
/*     File: Guesser.C                                             */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Jun 15 10:57:27 2007                              */
/* Modified: Fri Jul  6 15:19:32 2012 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/

#include <stdio.h>

#include "Guesser.h"



/*******************************************************************/
/*                                                                 */
/*  Guesser::build_suffix_trees                                    */
/*                                                                 */
/*******************************************************************/

void Guesser::build_suffix_trees(BaseLexicon &lexicon, vector<bool> &is_oc_tag,
				 vector<bool> &is_oc_tag_uc,
				 double threshold, int msl, double mlp)
{
  sufflex.resize( wordclass.number_of_classes );

  // collect frequency information from the lexicon
  for( BaseLexicon::iterator it=lexicon.begin(); it!=lexicon.end(); it++ ) {
    const char *word = it->first;
    int wc = wordclass.number( word );
    Entry &entry = lexicon.get_entry(it->second);
    double sum = 0;
    for( size_t i=0; i<entry.tag.size(); i++ )
      sum += entry.tag[i].freq;
    if (sum > 0.0) {
      entry.freq = (float)sum;
      sufflex[wc].add( is_oc_tag, is_oc_tag_uc, entry, word, msl );
    }
  }

  // estimate the tag probabilities
  for( size_t i=0; i<sufflex.size(); i++ ) {
    if (sufflex[i].number_of_tags() == 0)
      fprintf(stderr,"Warning: Word class %d did not occur!\n", (int)i);
    else
      sufflex[i].prune( threshold, mlp );
  }
}


/*******************************************************************/
/*                                                                 */
/*  Guesser::Guesser                                               */
/*                                                                 */
/*******************************************************************/

Guesser::Guesser( FILE *wcfile, FILE *ocf, FILE *ocf2, SymbolTable &tagmap,
		  BaseLexicon &lexicon, double threshold, int msl, double mlp)
  : wordclass( wcfile, textfile )
{
  char buffer[10000];
  
  // read the set of open class POS tags from a file
  if (ocf) {
    is_oc_tag.resize( tagmap.size(), false );
    while (fscanf(ocf, "%s", buffer) == 1) {
      SymNum tagnum;
      if (tagmap.lookup( buffer, tagnum ))
	is_oc_tag[tagnum] = 1;
      else
	warnx("Warning: open class file contains the unknown POS tag \"%s\"", 
	      buffer);
    }
  }
  else
    is_oc_tag.resize( tagmap.size(), true );

  if (ocf2) {
    is_oc_tag_uc.resize( tagmap.size(), false );
    while (fscanf(ocf2, "%s", buffer) == 1) {
      SymNum tagnum;
      if (tagmap.lookup( buffer, tagnum ))
	is_oc_tag_uc[tagnum] = 1;
      else
	warnx("Warning: open class file contains the unknown POS tag \"%s\"", 
	      buffer);
    }
  }
  else
    is_oc_tag_uc = is_oc_tag;

  build_suffix_trees( lexicon, is_oc_tag, is_oc_tag_uc, threshold, msl, mlp );
}
