
/*******************************************************************/
/*                                                                 */
/*     File: Lexicon.h                                             */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Feb 20 17:04:52 2009                              */
/* Modified: Fri Jul  6 15:22:02 2012 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#ifndef __LEXICON
#define __LEXICON

#include "LexSmoother.h"

// smoothing strategies
// The tag probability estimator of BaseLexicon does Witten-Bell smoothing
// with the Prior probabilities of the tags.
// LexSmoother 

/*****************  class Lexicon  *********************************/

class Lexicon {

private:

public:
  SymbolTable &symbols;
  BaseLexicon baselexicon;
  Guesser guesser;
  
  Lexicon( FILE *lexfile, FILE *ocf, FILE *ocf2, FILE *wcfile, 
	   SymbolTable &symtab, double SmoothingWeight, double MinLexProb,
	   double GuesserGain, int msl, bool wl );
  
  Entry *lookup( const char *word, bool sstart );
  
  void multiply_prior_and_term_prob();

  void print( FILE *file ) const {
    baselexicon.print( symbols, file );
    guesser.print( symbols, file );
  }

  void write( FILE *file ) {
    baselexicon.write( file, symbols );
  }
};

#endif
