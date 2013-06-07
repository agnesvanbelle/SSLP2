
/*MA****************************************************************/
/*                                                                 */
/*     File: vitparser.h                                           */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Mon Dec 23 09:40:44 2002                              */
/* Modified: Thu Oct 25 16:39:11 2012 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include "baseparser.h"

/*****************  class VParse  **********************************/

class VParse {
public:
  SymNum symbol;
  bool termp;
  union {
    struct {
      int rule_number;
      VParse *left;
      VParse *right;
    } nterm;
    struct {
      char *word;
    } term;
  } data;
  
  VParse() {};

  VParse( SymNum s, char *w ) {
    termp = true; 
    symbol = s; 
    data.term.word = w; 
  };

  VParse( SymNum s, int rn, VParse *l, VParse *r=NULL ) {
    termp = false;
    symbol = s; 
    data.nterm.rule_number = rn; 
    data.nterm.left = l; 
    data.nterm.right = r;
  };

  ~VParse() {
    if (!termp) {
      delete data.nterm.left;
      if (data.nterm.right)
	delete data.nterm.right;
    }
  }
};


/*****************  class VitParser  *******************************/

class VitParser : public BaseParser {

private:

  vector<VParse*> parse;
  vector<Prob>    vitprob;

  VParse *build_subtree( SymNum s, size_t b, size_t e, Index n );
  VParse *build_subtree2( SymNum s, size_t b, size_t e, Index n, vector<char>& );
  void print_trace( int, size_t&, size_t&, FILE* );
  void print_subtree( VParse*, int, size_t&, size_t&, FILE* );

public:
  bool PrintProbs;
  bool MaxFScore;

  VitParser( FILE *gfile, FILE *lfile, char *ss, FILE *ocf, FILE *ocf2, 
	     FILE *wcf, char tss, char tes, double sw, double mlp, 
	     double gt, int msl, bool wl, bool pst ) :
  BaseParser( gfile, lfile, ss, ocf, ocf2, wcf, tss, tes, sw, mlp, gt, msl, wl, pst )
    { PrintProbs = MaxFScore = false; }
    
  bool next_parse( FILE* );
  void print_parse( FILE* );

  void clear() {
    for( size_t i=0; i<parse.size(); i++ )
      delete parse[i];
    parse.clear();
    vitprob.clear();
  }
};
