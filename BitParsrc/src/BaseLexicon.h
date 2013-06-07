
/*******************************************************************/
/*                                                                 */
/*     File: BaseLexicon.h                                         */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jun 14 16:23:17 2007                              */
/* Modified: Tue Aug 16 16:32:28 2011 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#ifndef LEXICON_H
#define LEXICON_H

#include <assert.h>

#include "SymbolTable.h"
#include "Entry.h"



/*****************  class BaseLexicon  *****************************/

class BaseLexicon {

private:
  SymbolTable wordtab;
  vector<Entry> entry;

  Entry &add_word( const char *word ) {
    size_t n = wordtab.number( word );
    if (n == entry.size()) // new word?
      entry.resize(n+1); // add a new entry
    return entry[n];
  }

  void compute_priors();
  void extract_terminals();

public:
  vector<double> TagFreq;
  vector<double> PriorProb;
  vector<double> TermProb;
  vector<SymNum> terminal_symbol;

  BaseLexicon( FILE *file, SymbolTable &symbols, bool withlemma );

 BaseLexicon( FILE *file ) : wordtab(file) {
    read_datavec( entry, file );
    extract_terminals();
  }

  void store( FILE *file ) const {
    // Store the hash table with the words
    wordtab.store(file);
    write_datavec( entry, file );
  }
  
  void print( SymbolTable &symtab, FILE *file ) const {
    fprintf(file,"\nLexicon with word frequencies and POS tag probabilities\n");
    fprintf(file,"-------------------------------------------------------\n");
    for( SymNum n=0; n<wordtab.size(); n++ ) {
      fprintf(file,"%s ", wordtab.name(n));
      entry[n].print(symtab,file);
    }
  }
  
  Entry *lookup( const char *word ) {
    SymbolTable::iterator it = wordtab.find(word);
    if (it == wordtab.end())
      return NULL;
    return &entry[it->second];
  }


  double multiply_prior_and_term_prob();

  Entry &get_entry( size_t n  ) { return entry[n]; }

  typedef SymbolTable::iterator iterator;

  iterator find( const char *s ) { return wordtab.find(s); }
  iterator begin() { return wordtab.begin(); }
  iterator end()   { return wordtab.end(); }
  size_t   size()  { return wordtab.size(); }

  void write( FILE *file, SymbolTable &symbols );
};

#endif
