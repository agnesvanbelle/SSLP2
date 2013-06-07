/*******************************************************************/
/*      File: parser.h                                             */
/*    Author: Helmut Schmid                                        */
/*   Purpose:                                                      */
/*   Created: Tue Nov  5 09:36:57 2002                             */
/*  Modified: Thu Oct 25 16:40:04 2012 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include <limits.h>

#include "baseparser.h"

#define TERMBASE INT_MAX
#define UNARYBASE -1

typedef unsigned short HeadIndex;
static const HeadIndex MaxHeadIndex=USHRT_MAX;


/*****************  class Parse  ***********************************/

class Parse {

  class TermInfo {
  public:
    Index word_pos; // position of the word in the sentence
    Index tag_pos;  // position of the tag in the word's list of POS tags
    HeadIndex head; // position of the lexical head in the head_item table

    TermInfo( Index w, Index t ) { word_pos = w; tag_pos = t; };
  };

 public:

  /***  class Parse::HeadItem  ***/

  class HeadItem {

  public:
    const char *lemma;
    SymNum     symbol;
    HeadItem( const char *h, SymNum c ) : lemma(h), symbol(c) {}

    bool operator==( const HeadItem &h ) const { 
      return (h.lemma == lemma && h.symbol == symbol);
    }

    bool operator!=( const HeadItem &h ) const { 
      return (h.lemma != lemma || h.symbol != symbol);
    }
  };

  int number_of_roots;

  // tables for storing nodes
  vector<SymNum> symbol;
  vector<HeadIndex> head;
  vector<Index> first_analysis;

  // tables for storing edges
  vector<int> rule_data;
  vector<Index> first_daughter;

  // table for storing links
  vector<Index> daughter;

  // table with information about terminal nodes
  vector<TermInfo> term_info;

  // table with information about the possible lexical heads
  vector<HeadItem> head_item;

  int terminal_rule_data( Index wpos, Index tpos ) {
    int result = (int)(TERMBASE - term_info.size());
    term_info.push_back(TermInfo(wpos, tpos));
    return result;
  };

  int unary_rule_data( unsigned int index ) {
    return UNARYBASE - (int)index;
  };

  int binary_rule_data( unsigned int index ) {
    return (int)index;
  };

  size_t number_of_nodes() { return symbol.size(); };

  size_t number_of_edges() { return rule_data.size(); };

  size_t number_of_analyses( Index n )
    { return first_analysis[n+1] - first_analysis[n]; };

  Index add_node( SymNum c, HeadIndex h ) {
    size_t n = symbol.size();
    symbol.push_back( c );
    head.push_back( h );
    first_analysis.push_back((unsigned)first_daughter.size());
    return (Index)n;
  };

  void clear() { 
    vector<SymNum>().swap(symbol); 
    vector<HeadIndex>().swap(head);
    vector<Index>().swap(first_analysis);
    vector<int>().swap(rule_data);
    vector<Index>().swap(first_daughter);
    vector<Index>().swap(daughter);
    vector<TermInfo>().swap(term_info);
    vector<HeadItem>().swap(head_item);
  };
};


class Parser;
class Edge;

/*****************  class Node  ************************************/

class Node {

 private:
  Index n;

 public: 

  /***  class Node::iterator  ***/

  class iterator {
    
  private:
    Index n;
    Index nn;
    
  public:
    static Parser *parser;

    inline iterator( Index index, Index nodeindex ) 
      : n(index), nn(nodeindex) {}

    inline iterator &operator++() { n++; return *this; }

    inline bool operator==( const iterator &it ) const { return n == it.n; }

    inline bool operator!=( const iterator &it ) const { return n != it.n; }

    inline Edge operator*();
  };

  /*** end of class Node::iterator  ***/
  
  inline Node( Index pos ) : n(pos) {};

  inline Node() : n(0) {};

  inline SymNum category() const;

  inline const char *symbol_name() const;

  inline const HeadIndex head_index() const;

  inline const Parse::HeadItem &head_item() const;

  inline const char *head_string() const;

  inline SymNum head_tag() const;

  inline const char *head_tag_name() const;

  inline Parser *my_parser() { return iterator::parser; };

  inline Index number() const { return n; };

  inline Edge edge( int i );

  inline bool is_aux() const;

  inline bool is_ambiguous();

  inline size_t size() const;
  inline iterator begin();
  inline iterator end();

  inline Prob &prob() const;
  inline Prob &freq() const;

  bool operator<( const Node &n ) const { return prob() > n.prob(); }
};


/*****************  class Edge  ************************************/

class Edge {
  
private:
  Index  n;
  Index  nn;

public:

  /***  class Edge::iterator  ***/

  class iterator {
    
  private:
    Index n;

  public:
    static Parser *parser;

    inline iterator( Index pos ) : n(pos) {};
    
    inline bool operator==( const iterator &it ) const { return n == it.n; };
    
    inline bool operator!=( const iterator &it ) const { return n != it.n; };
    
    inline Node operator*();
    
    inline iterator &operator++() { n++; return *this; };
  };

  /***  end of class Edge::iterator  ***/
  
  inline int rule_data() const;
   
  inline Edge( Index index, Index nodeindex ) : n(index), nn(nodeindex) {};
  
  inline Edge() {};
  
  inline Node parent() const;
  
  inline SymNum category() const;
  
  inline int unary_rule_index() const;

  inline int binary_rule_index() const;

  inline ChainRule &unary_rule() const;

  inline NFRule &binary_rule() const;

  inline RuleNumber source_rule_number() const;

  inline double rule_prob() const;
  
  inline void incr_rule_freq() const;

  inline unsigned short headpos() const;

  inline bool is_terminal() const;
  
  inline bool is_unary() const;
  
  inline bool is_binary() const;

  inline const char *word() const;
  
  inline Index word_pos() const;
  inline Index tag_pos() const;
  inline HeadIndex head() const;
  
  inline Node node( int i ) const;
	 
  inline int term_info_index() const;
  
  inline Index number() const { return n; };
  
  inline Prob &prob() const;
  inline Prob &freq() const;

  inline void print_rule();
  
  inline Parser *my_parser() { return iterator::parser; };
  
  inline iterator begin();
  inline iterator end();

  bool operator<( const Edge &e ) const { return prob() > e.prob(); }
};


/*****************  class Parser  **********************************/

class Parser : public BaseParser {

 private:
  vector<Prob> NodeProb;
  vector<Prob> EdgeProb;
  vector<Prob> NodeFreq;
  vector<Prob> EdgeFreq;

  void prune_analyses( SymNum cat, size_t b, size_t e, vector<Analysis>& );
  Index build_parse( SymNum, size_t, size_t, NodeIndexTab& );
  void print_daughters( Node, vector<int>&, vector<int>&, size_t&, 
			int, size_t&, size_t&, vector<int>&, FILE* );
  void print_node( Node, vector<int>&, FILE* );
  void print_best_node( Node, int, size_t&, size_t&, FILE* );

  void mark_daughters( Node, vector<int>&, vector<int>&, size_t&, 
		       vector<int>&);
  void mark_node( Node, vector<int>& );

  void inside();
  void viterbi();
  void estimate_freqs();
  void train();

  int add_trace_prob_edge(Edge&, int, int, vector<Trace>&, int, Prob);
  int add_trace_prob( Node, int );

  void apply( void (*npre)(Node), void (*npost)(Node), 
	      void (*epre)(Edge&)=NULL, void (*epost)(Edge&)=NULL, 
	      void (*apre)(Edge&)=NULL, void (*apost)(Edge&)=NULL, 
	      void (*rpre)(Node)=NULL, void (*rpost)(Node)=NULL ); 

  bool po_apply( void (*npre)(Node), void (*npost)(Node), 
		 void (*epre)(Edge&)=NULL, void (*epost)(Edge&)=NULL, 
		 void (*apre)(Edge&)=NULL, void (*apost)(Edge&)=NULL, 
		 void (*rpre)(Node)=NULL, void (*rpost)(Node)=NULL);

  void create_head_item_table();

 public:
  Parse parse;

  /***  class Parser::iterator  ***/

  class iterator {
    
  private:
    Index n;
    
  public:
    inline iterator( Index pos ) : n(pos) {};
    
    inline bool operator==( const iterator &it ) const { return n == it.n; };
    
    inline bool operator!=( const iterator &it ) const { return n != it.n; };
    
    inline Node operator*();
    
    inline iterator &operator++() { n++; return *this; };
  };

  /***  end of class Edge::iterator  ***/

  inline iterator begin();
  inline iterator end();


  bool Lexicalized;
  bool Viterbi;     // true for lexicalized Viterbi parsing
  bool ViterbiProbs;
  bool InsideProbs;
  bool EstimatedFreqs;
  bool Training;
  bool PrintRuleNumbers;
  bool PrintRuleProbs;
  size_t  NBest;
  Prob PruningThreshold;

 Parser( FILE *gfile, FILE *lfile, char *ss, FILE *ocf, FILE *ocf2, FILE *wcf, 
	 char tss, char tes, double sw, double mlp, double gt,
	 int msl, bool wl, bool pst ) :
  BaseParser( gfile, lfile, ss, ocf, ocf2, wcf, tss, tes, sw, mlp, gt, msl, wl, pst )
    {
      Lexicalized      = false;
      Viterbi          = false;
      ViterbiProbs     = false;
      InsideProbs      = false;
      EstimatedFreqs   = false;
      PrintRuleNumbers = false;
      PrintRuleProbs   = false;
      NBest            = 0;
      PruningThreshold = (Prob)0.0;
      Edge::iterator::parser = this;
      Node::iterator::parser = this;
    };

  Parse *next_parse( FILE *infile );
  void lexicalize_parse();
  void print_parse( FILE *file );
  void print_parse_tables( Parse& );
  void print_parse_tables() { print_parse_tables( parse ); }
  void print_best_parse( FILE *file );
  void print_nbest_parses( FILE *file );
  void print_YAP_parse( FILE *file );
  void print_trace( int rn, int &tpos, int dpos, FILE *file );
  void print_trace_probs( FILE *file );

  void clear() {
    parse.clear();
    // clear the following vectors and reduce their capacity to 0
    vector<Prob>().swap(NodeProb);
    vector<Prob>().swap(EdgeProb);
    vector<Prob>().swap(NodeFreq);
    vector<Prob>().swap(EdgeFreq);
  };

  friend class Node;
  friend class Edge;
  friend class Edge::iterator;
};


/******************  Parse member functions  ***********************/

inline Node Parser::iterator::operator*() { 
  return Node( n );
}

inline Parser::iterator Parser::begin() { 
  return iterator( 0 );
}
 
inline Parser::iterator Parser::end() { 
  return iterator( parse.number_of_roots );
}


/******************  Node member functions  ************************/

inline Edge Node::iterator::operator*() {
  return Edge(n, nn);
}

inline SymNum Node::category() const {
  return iterator::parser->parse.symbol[n];
}

inline Edge Node::edge( int i ) {
  return Edge( iterator::parser->parse.first_analysis[n] + i, n );
}

inline bool Node::is_aux() const { 
  return (category() >= (unsigned)iterator::parser->grammar.number_of_symbols());
}

inline Node::iterator Node::begin() { 
  return iterator( iterator::parser->parse.first_analysis[n], n );
}

inline Node::iterator Node::end() {
  return iterator( iterator::parser->parse.first_analysis[n+1], n );
}

inline size_t Node::size() const { 
  return (size_t)iterator::parser->parse.number_of_analyses(n);
}

inline Prob &Node::prob() const { 
  return iterator::parser->NodeProb[n];
}

inline Prob &Node::freq() const { 
  return iterator::parser->NodeFreq[n];
}

inline const char *Node::symbol_name() const { 
  return iterator::parser->nfg.symbol_name(category());
}

inline const HeadIndex Node::head_index() const { 
  return iterator::parser->parse.head[n];
}

inline const Parse::HeadItem &Node::head_item() const { 
  return iterator::parser->parse.head_item[head_index()];
}

inline const char *Node::head_string() const { 
  return iterator::parser->parse.head_item[head_index()].lemma;
}

inline SymNum Node::head_tag() const { 
  return iterator::parser->parse.head_item[head_index()].symbol;
}

inline const char *Node::head_tag_name() const { 
  return iterator::parser->grammar.symbol_name(head_tag());
}

inline bool Node::is_ambiguous() {
  if (size() > 1)
    return true;
  
  iterator e=end();
  for( iterator it=begin(); it!=e; ++it ) {
    Edge edge = *it;
    Edge::iterator e2=edge.end();
    for( Edge::iterator it=edge.begin(); it!=e2; ++it ) {
      Node daughter=*it;
      if (daughter.is_aux() && daughter.is_ambiguous())
	return true;
    }
  }
  return false;
}


/******************  Edge member functions  ***********************/

inline int Edge::rule_data() const { 
  return iterator::parser->parse.rule_data[n];
}

inline bool Edge::is_terminal() const { 
  size_t n = iterator::parser->parse.term_info.size();
  return (rule_data() > TERMBASE - (int)n);
}

inline bool Edge::is_unary() const { 
  return (rule_data() <= UNARYBASE);
}

inline bool Edge::is_binary() const { 
  return !(is_unary() || is_terminal());
}

inline Edge::iterator Edge::begin() { 
  return iterator( iterator::parser->parse.first_daughter[n] );
}
 
inline Edge::iterator Edge::end() { 
  return iterator( iterator::parser->parse.first_daughter[n+1]);
}

inline Node Edge::iterator::operator*() { 
  return Node( iterator::parser->parse.daughter[n] );
}

inline const char *Edge::word() const {
  return iterator::parser->word[iterator::parser->parse.term_info[TERMBASE-rule_data()].word_pos];
}

inline Index Edge::word_pos() const {
  return iterator::parser->parse.term_info[TERMBASE-rule_data()].word_pos;
}

inline Index Edge::tag_pos() const {
  return iterator::parser->parse.term_info[TERMBASE-rule_data()].tag_pos;
}

inline HeadIndex Edge::head() const {
  return (HeadIndex)iterator::parser->parse.term_info[TERMBASE-rule_data()].head;
}

inline Prob &Edge::prob() const { 
  return iterator::parser->EdgeProb[number()];
}

inline Prob &Edge::freq() const {
  return iterator::parser->EdgeFreq[number()];
}

inline Node Edge::node( int i ) const {
  Index m = iterator::parser->parse.first_daughter[n]+i;
  return Node( iterator::parser->parse.daughter[m] );
}

inline Node Edge::parent() const {
  return Node(nn);
}

inline int Edge::unary_rule_index() const { 
  return UNARYBASE - rule_data();
}

inline int Edge::binary_rule_index() const { 
  return rule_data();
}

inline SymNum Edge::category() const {
  return iterator::parser->parse.symbol[nn];
}

inline ChainRule &Edge::unary_rule() const {
  return iterator::parser->nfg.chain[category()].down[unary_rule_index()];
}

inline NFRule &Edge::binary_rule() const {
  return iterator::parser->nfg.get_rules(category())[binary_rule_index()];
}

inline RuleNumber Edge::source_rule_number() const {
  if (is_terminal())
    return -1;
  if (is_unary())
    return unary_rule().source_rule;
  if (is_binary())
    return binary_rule().source_rule;
  assert(0);
  throw "in function Edge::source_rule_number()";
}

inline double Edge::rule_prob() const { 
  if (is_terminal())
    return iterator::parser->tags[word_pos()].tag[tag_pos()].prob;

  return iterator::parser->grammar.ruleprob[source_rule_number()];
}

inline void Edge::incr_rule_freq() const {
  double f = iterator::parser->EdgeFreq[n];
  if (is_terminal())
    iterator::parser->tags[word_pos()].tag[tag_pos()].freq += (float)f;

  RuleNumber rn = source_rule_number();
  if (rn != -1) // not an auxiliary rule
    iterator::parser->grammar.incr_freq( rn, f );
}

inline void Edge::print_rule() {
  Node node = parent();
  fprintf(stderr, "%s(%d) ->", node.symbol_name(), node.number());
  if (is_terminal())
    fprintf(stderr, " %s", word());
  for( iterator it=begin(); it!=end(); ++it ) {
    node = *it;
    fprintf(stderr, " %s(%d)", node.symbol_name(), node.number());
  }
}
