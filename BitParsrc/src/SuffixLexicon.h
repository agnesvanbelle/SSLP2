
/*******************************************************************/
/*                                                                 */
/*     File: SuffixLexicon.h                                       */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Sep 13 14:02:07 2007                              */
/* Modified: Fri Jul  6 15:17:23 2012 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#include "SymbolTable.h"
#include "io.h"
#include "Entry.h"

class SLink;

/*****************  class SEntry  **********************************/

class SEntry : public Entry {

 public:
  float restfreqsum;
  vector<float> restfreq;
  vector<float> restprob;

  void store( FILE *file ) const {
    Entry::store( file );
  }

  void print( SymbolTable &st, FILE *file ) const {
    Entry::print( st, file );
  }

 SEntry() {}

 SEntry( FILE *file ) : Entry( file ) {}
};


/*****************  class SNode  ***********************************/

class SNode {

 private:

  vector<SLink> link;

  void subtract_frequencies( SNode &daughter );

  void estimate_probs( double mlp, vector<double> &prob );

 public:
  SEntry entry;
  SNode() {};

  void add( vector<bool> &is_oc_tag, vector<bool> &is_oc_tag_uc, 
	    const Entry &entry, const char *first, const char *last,
	    bool is_upper );
  void prepare();

  void prune( double threshold, char *suffix /*???*/);

  void estimate_probs( double mlp );

  void store( FILE *file ) const;

 SNode( FILE *file ) : entry( file ) {
    read_datavec( link, file );
  };

  Entry &lookup( const char *word, int pos );

  void print( SymbolTable &symtab, FILE *file, char *buffer, int pos ) const;

  bool is_empty() { return (link.size() + entry.tag.size() == 0); }

  void multiply_prior_and_term_prob( vector<double>&, vector<double>&, double);
};


/*****************  class SLink  ***********************************/

class SLink {

public:
  SNode node;
  bool active;
  char symbol;

  SLink( char s ) { active = false; symbol = s; }

  void store( FILE *file ) const {
    node.store( file );
    fputc( symbol, file );
  }

 SLink( FILE *file ) : node( file ) {
    symbol = (char)fgetc( file );
  }
};


/*****************  class SuffixLexicon  ***************************/

class SuffixLexicon {

  SNode root;

public:
  SuffixLexicon() {};

 SuffixLexicon( FILE *file ): root( file ) {}

  void add(vector<bool> &is_oc_tag, vector<bool> &is_oc_tag_uc, 
	   const Entry &e, const char *word, int msl);

  void prune( double threshold, double minlexprob ) { 
    char buffer[1001];
    buffer[1000] = 0;
    root.prepare();
    root.prune( threshold, buffer+1000 );
    root.estimate_probs( minlexprob );
  }

  size_t number_of_tags() { return root.entry.tag.size(); };

  void store( FILE *file ) const { root.store(file); }

  void print( SymbolTable &symtab, FILE *file ) const { 
    char buffer[1000];
    root.print( symtab, file, buffer, 0);
  }

  Entry &lookup( const char *word ) {
    return root.lookup(word, (int)strlen(word)-1);
  }

  void multiply_prior_and_term_prob( vector<double> &priorprob, 
				     vector<double> &termprob, double mwp )
  {
    root.multiply_prior_and_term_prob( priorprob, termprob, mwp );
  }

  bool is_empty() { return root.is_empty(); }
};

