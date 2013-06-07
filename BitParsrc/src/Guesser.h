
/*******************************************************************/
/*                                                                 */
/*     File: Guesser.h                                             */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Jun 15 10:57:37 2007                              */
/* Modified: Mon Jul  9 10:44:56 2012 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#ifndef GUESSER_H
#define GUESSER_H

#include "WordClass.h"
#include "BaseLexicon.h"
#include "SuffixLexicon.h"
#include "Guesser.h"


/*****************  class Guesser  *********************************/

class Guesser {

private:
  vector<SuffixLexicon> sufflex;

  void build_suffix_trees( BaseLexicon&, vector<bool> &is_oc_tag, 
			   vector<bool> &is_oc_tag_uc, 
			   double threshold, int msl, double mlp );

public:
  vector<bool> is_oc_tag;
  vector<bool> is_oc_tag_uc;
  WordClass wordclass;
  Guesser( FILE *wcfile, FILE *ocf, FILE *ocf2, SymbolTable&, BaseLexicon&, 
	   double threshold, int max_suffix_length, double mlp );

  void print( SymbolTable &symtab, FILE *file ) const {
    fprintf(file,"\nSuffix Lexicons\n");
    fprintf(file,"-------------------------------------------------------\n");
    for( size_t i=0; i<sufflex.size(); i++ ) {
      fprintf(file,"\nwordclass %u\n", (unsigned)i);
      sufflex[i].print(symtab, file);
    }
  }

  Entry *lookup( const char *word ) {
    int n = wordclass.number(word);
    if (sufflex[n].is_empty())
      return &sufflex[0].lookup(word);
    return &sufflex[n].lookup(word);
  }

  void multiply_prior_and_term_prob( vector<double> &priorprob, 
				     vector<double> &termprob, double mwp )
  {
    for( size_t i=0; i<sufflex.size(); i++ )
      sufflex[i].multiply_prior_and_term_prob(priorprob, termprob, mwp);
  }
};

#endif
