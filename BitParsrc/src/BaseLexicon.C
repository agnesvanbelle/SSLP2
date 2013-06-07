
/*******************************************************************/
/*                                                                 */
/*     File: BaseLexicon.C                                         */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jun 14 16:39:01 2007                              */
/* Modified: Tue Aug 16 16:33:47 2011 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/

#include <iostream>
using std::cerr;

#include "BaseLexicon.h"

extern bool Quiet;


/*******************************************************************/
/*                                                                 */
/*  BaseLexicon::compute_priors                                    */
/*                                                                 */
/*******************************************************************/

void BaseLexicon::compute_priors()

{
  // count the tags
  for( size_t i=0; i<entry.size(); i++ ) {
    vector<Tag> &tag = entry[i].tag;
    for( size_t k=0; k<tag.size(); k++ ) {
      if (TagFreq.size() <= tag[k].number)
	TagFreq.resize(tag[k].number+1, 0);
      TagFreq[tag[k].number] += tag[k].freq;
    }
  }
  
  double N=0;
  for( size_t i=0; i<TagFreq.size(); i++ )
    N += TagFreq[i];

  PriorProb.resize(TagFreq.size());
  for( size_t t=0; t<TagFreq.size(); t++ )
    PriorProb[t] = TagFreq[t] / N;
}


/*******************************************************************/
/*                                                                 */
/*  BaseLexicon::multiply_prior_and_term_prob                      */
/*                                                                 */
/*******************************************************************/

double BaseLexicon::multiply_prior_and_term_prob()

{
  double sum = 0.0;
  for( size_t i=0; i<entry.size(); i++ )
    sum += entry[i].freq;

  double min_word_prob = 0.1 / sum;
  for( size_t i=0; i<entry.size(); i++ ) {
    double wordprob = entry[i].freq / sum;
    if (wordprob < min_word_prob)
      wordprob = min_word_prob;
    for( size_t k=0; k<entry[i].tag.size(); k++ ) {
      Tag &tag = entry[i].tag[k];
      tag.prob = (float)(TermProb[tag.number]  * tag.prob * wordprob / 
			 PriorProb[tag.number]);
    }
  }
  return min_word_prob;
}


/*******************************************************************/
/*                                                                 */
/*  BaseLexicon::extract_terminals                                 */
/*                                                                 */
/*******************************************************************/

typedef hash_set<SymNum> IH;

void BaseLexicon::extract_terminals()

{
  IH tags;
  for( size_t i=0; i<entry.size(); i++ )
    for( size_t k=0; k<entry[i].tag.size(); k++ )
      if (tags.find(entry[i].tag[k].number) == tags.end()) 
       	tags.insert(entry[i].tag[k].number);

  terminal_symbol.reserve(tags.size());
  for( IH::iterator i=tags.begin(); i!=tags.end(); i++ )
    terminal_symbol.push_back( *i );
}


/*******************************************************************/
/*                                                                 */
/*  BaseLexicon::BaseLexicon                                       */
/*                                                                 */
/*******************************************************************/

BaseLexicon::BaseLexicon( FILE *file, SymbolTable &symbols, bool withlemma )

{
  if (!Quiet)
    cerr << "reading the lexicon...";

  char buffer[100000];
  unsigned long l;

  for( l=0; fgets(buffer, 100000, file); l++ ) {
    char *tag, *word = strtok(buffer, "\t");
    while ((tag = strtok(NULL," \t\n"))) {
      char *freq = strtok(NULL," \t\n");
      char *lemma=word;

      if (freq == NULL) {
	fprintf(stderr,"Error in line %ld of lexicon!\n", l);
	exit(1);
      }

      if (withlemma)
	lemma = strtok(NULL,"\t\n");


      Tag &t = add_word(word).add_tag(symbols.number(tag), lemma);
      t.freq = (float)atof(freq);
    }
  }

  compute_priors();
  
  // estimate the probabilities
  for( size_t i=0; i<entry.size(); i++ )
    // The tags are sorted in the next step
    entry[i].estimate_tag_probs( 0.0, &PriorProb ); // smooth the probabilities

  extract_terminals();

  if (!Quiet)
    cerr << "finished\n";
}


/*******************************************************************/
/*                                                                 */
/*  BaseLexicon::write                                             */
/*                                                                 */
/*******************************************************************/

void BaseLexicon::write( FILE *file, SymbolTable &symbols )

{
  for( iterator it=begin(); it!=end(); it++ ) {
    const char *word = it->first;
    Entry &e = entry[it->second];
    
    for( size_t i=0; i<e.tag.size(); i++ ) {
      Tag &tag = e.tag[i];
      fprintf(file,"%s\t%s %f", word, symbols.name(tag.number), tag.freq);
      if (tag.lemma != NULL)
	fprintf(file," %s", tag.lemma);
      fputc('\n', file);
    }
  }
}
