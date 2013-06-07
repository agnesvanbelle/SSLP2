
/*******************************************************************/
/*                                                                 */
/*     File: Entry.h                                               */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jun 14 16:23:17 2007                              */
/* Modified: Fri Oct 19 14:43:19 2012 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#ifndef _ENTRY_H
#define _ENTRY_H

#include "SymbolTable.h"
#include "StringSet.h"
#include "io.h"

static StringSet RefStringLemma;


/*****************  class Tag  *************************************/

class Tag {

public:
  SymNum number;  
  float prob;
  float freq;
  const char *lemma;

  Tag() { lemma = NULL; }  // resize wants this
  
  Tag( SymNum n, const char *lem=NULL, float f=0.0 ) {
    number = n;
    prob = 0.0;
    freq = f;
    if (lem == NULL)
      lemma = NULL;
    else
      lemma = RefStringLemma(lem);
  }

  Tag ( FILE *file ) {
    read_data(number, file);
    read_data(prob, file);
    char *lem = read_string(file);
    if (strcmp(lem,"") == 0)
      lemma = NULL;
    else
      lemma = RefStringLemma(lem);
    free(lem);
    freq = 0.0;
  }

  void store( FILE *file ) const {
    write_data(number, file);
    write_data(prob, file);
    if (lemma == NULL)
      write_string("", file);
    else
      write_string(lemma, file);
  }

  inline int operator<(const Tag tag) const { // for sorting
    return (number < tag.number);
  }
};


/*****************  class Entry  ***********************************/

class Entry {

public:
  float freq;
  vector<Tag> tag;

  Entry() { freq = 0.0; }

  Entry( FILE *file ) {
    read_data( freq, file );
    read_datavec( tag, file );
  }

  void sort_tags() { sort(tag.begin(), tag.end()); }

  void store( FILE *file ) const {
    write_data( freq, file );
    write_datavec( tag, file );
  }

  void print( FILE *file ) const {
    fprintf(file,"(%.2f): ", freq);
    for( size_t i=0; i<tag.size(); i++ ) {
      fprintf(file," %d (%.3g)", tag[i].number, tag[i].prob);
      if (tag[i].lemma != NULL)
	fprintf(file," %s", tag[i].lemma);
    }
    fprintf(file,"\n");
  }

  void print( SymbolTable &symtab, FILE *file ) const {
    fprintf(file,"(%.2f): ", freq);
    for( size_t i=0; i<tag.size(); i++ ) {
      fprintf(file," %s (%.3g)", symtab.name(tag[i].number), tag[i].prob);
      if (tag[i].lemma != NULL)
	fprintf(file," %s", tag[i].lemma);
    }
    fprintf(file,"\n");
  }

  Tag *find_tag( SymNum n ) {
    for( size_t i=0; i<tag.size(); i++ )
      if (tag[i].number == n)
	return &tag[i];
    return NULL;
  }

  Tag &add_tag( SymNum n, const char *lem=NULL ) {
    Tag *t = find_tag( n );
    if (t)
      return *t;
    tag.push_back(Tag(n, lem));
    return tag.back();
  }

  void estimate_tag_probs( double MinProb=0.0, vector<double> *PriorProb=NULL );

  size_t max_tag_index() {
    size_t N = 0;
    for( size_t i=0; i<tag.size(); i++ )
      if (N < tag[i].number)
	N = tag[i].number;
    return N;
  }
};

#endif
