
/*******************************************************************/
/*                                                                 */
/*     File: lexicalize.C                                          */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jun  8 15:23:54 2006                              */
/* Modified: Wed May  4 13:10:25 2011 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/

#include <limits.h>

#include <iostream>
using std::cerr;

#include "parser.h"

static bool *Visited;

typedef unsigned short HeadPos;
const HeadPos MaxHeadPos = USHRT_MAX;


vector<Index> first_lex_node;


/*****************  class LexInfo  *********************************/

class LexInfo {

  // list of head daughters for each temporary node
  vector< vector<Index> > headdaughter;

  // information about terminal nodes and their heads
  vector<Index> term_node;
  vector<HeadIndex> term_node_head;

  // bit vectors for the representation of the head information
  vector<unsigned long*> headvec;

  void add_daughter( Index p, Index d ) {
    vector<Index> &head = headdaughter[p];
    for( size_t i=0; i<head.size(); i++ )
      if (head[i] == d)
	return; // the head was inserted before
    head.push_back(d);
  };

 public:
  size_t number_of_heads;
  size_t headvec_length;

  void extract_heads( Node node );
  void propagate_heads( Parse &parse );

  LexInfo( size_t n ) : headdaughter(n) {}

  ~LexInfo() {
    for( size_t i=0; i<headvec.size(); i++ )
      delete[] headvec[i];
  }

  void heads_for( Index n, vector<size_t> &heads ) {
    heads.clear();
    extract_bits( 0, number_of_heads, headvec[n], heads);
  }
};


/*****************  class AnalysisCount  ***************************/

class AnalysisCount {
  
public:
  vector<int> head;
  vector<int> number_of_analyses;
  vector<int> number_of_daughters;
  
  AnalysisCount( Node node, Parse &lp ) {
    int n = node.number();
    assert(first_lex_node[n] < first_lex_node[n+1]);
    for( Index i=first_lex_node[n]; i<first_lex_node[n+1]; i++ )
      head.push_back( lp.head[i] );
    number_of_analyses.resize( head.size(), 0 );
    number_of_daughters.resize( head.size(), 0 );
  };
  
  void add( Edge &edge, int nd, HeadIndex h ) {
    for( size_t i=0; i<head.size(); i++ )
      if (head[i] == h) {
	number_of_analyses[i]++;
	number_of_daughters[i] += nd;
	return;
      }
    assert(0);
    throw "in function AnalysisCount::add()";
  };
  
  void add( Edge &edge, int nd, Parse &lp, Index from, Index to ) {
    while (from < to)
      add( edge, nd, lp.head[from++] );
  };
  
  void add( Edge &edge, int nd, Parse &lp, Index f, Index to, 
	    Index f2, Index to2 )
  {
    while (f < to) {
      while (f2 < to2) {
	add( edge, nd, lp.head[f] );
	f2++;
      }
      f++;
    }
  };

  size_t number_of_heads() { return head.size(); };

  size_t total_number_of_analyses() {
    size_t n=0;
    for( size_t i=0; i<number_of_analyses.size(); i++ )
      n += number_of_analyses[i];
    return n;
  };

  size_t total_number_of_daughters() {
    size_t n=0;
    for( size_t i=0; i<number_of_daughters.size(); i++ )
      n += number_of_daughters[i];
    return n;
  };
};


/*******************************************************************/
/*                                                                 */
/*  Parser::create_head_item_table                                 */
/*                                                                 */
/*******************************************************************/

void Parser::create_head_item_table()

{
  // assign a head to each terminal node
  for( size_t i=0; i<parse.term_info.size(); i++ ) {
    size_t wp = parse.term_info[i].word_pos;
    size_t tp = parse.term_info[i].tag_pos;

    // look up the lemma in the lexicon
    Entry &POStag = tags[wp];
    const char *hs = POStag.tag[tp].lemma;
    if (hs == NULL)
      hs = word[wp];

    // add the new lemma to the list of heads
    Parse::HeadItem h( hs, POStag.tag[tp].number );
    for( HeadIndex k=0; ; k++ ) {
      if (k == parse.head_item.size())
	parse.head_item.push_back(h);   // add new head
      else if (parse.head_item[k] != h)
	continue;
      // else this head is already known
      parse.term_info[i].head = k;
      break;
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  LexInfo::extract_heads                                         */
/*                                                                 */
/*******************************************************************/

void LexInfo::extract_heads( Node node )

{
  if (Visited[node.number()])
    return;
  Visited[node.number()] = true;

  for( Node::iterator it=node.begin(); it!=node.end(); ++it ) {
    Edge edge=*it;
    
    if (edge.is_terminal()) {
      // store the head information of the terminal node
      term_node.push_back(node.number());
      term_node_head.push_back(edge.head());
    }
    else if (edge.is_unary()) {
      Node daughter = *edge.begin();
      extract_heads( daughter );
      add_daughter( node.number(), daughter.number() );
    }
    else {
      HeadDir dir = edge.binary_rule().headdir;
      Edge::iterator it=edge.begin();
      Node d1=*it;
      Node d2=*(++it);
      extract_heads( d1 );
      extract_heads( d2 );
      if (dir == left)
	add_daughter( node.number(), d1.number() );
      else
	add_daughter( node.number(), d2.number() );
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  LexInfo::propagate_heads                                       */
/*                                                                 */
/*******************************************************************/

void LexInfo::propagate_heads( Parse &parse )

{
  // allocate bit vectors for the computation of the head sets
  headvec.resize(headdaughter.size());
  number_of_heads = parse.head_item.size();
  headvec_length = bitarray_size( number_of_heads );
  for( size_t i=0; i<headvec.size(); i++ ) {
    headvec[i] = new unsigned long[headvec_length];
    for( size_t k=0; k<headvec_length; k++ )
      headvec[i][k] = 0;
  }

  // store the heads of terminal nodes in the respective bit vector
  for (size_t i=0; i<term_node.size(); i++ )
    set_bit( headvec[term_node[i]], term_node_head[i] );

  // propagate the head information to the ancestor nodes
  bool repeat;
  int N=0;
  do {
    N++;
    repeat = false;
    for( size_t i=0; i<headdaughter.size(); i++ ) {
      vector<Index> &daughter = headdaughter[i];
      unsigned long *pvec = headvec[i];
      for( size_t k=0; k<daughter.size(); k++ ) {
	unsigned long *dvec = headvec[daughter[k]];
	for( size_t l=0; l<headvec_length; l++ ) {
	  unsigned long old = pvec[l];
	  pvec[l] |= dvec[l];
	  if (pvec[l] != old)
	    repeat = true;
	}
      }
    }
  } while (repeat);

  if (Verbose)
    cerr << N << " iterations of head propagation\n";

  // clear the following vectors and reduce their capacity to 0
  vector<Index>().swap(term_node);
  vector<HeadIndex>().swap(term_node_head);
  vector< vector<Index> >().swap(headdaughter);
}


/*******************************************************************/
/*                                                                 */
/*  find_node_with_head                                            */
/*                                                                 */
/*******************************************************************/

static int find_node_with_head( Parse &lparse, Index n, HeadIndex h ) 

{
  HeadIndex *head = &(lparse.head[n]);
  for( int i=0; ; i++ )
    if (head[i] == h)
      return i;
  throw "in function find_node_with_head()";
}


/*******************************************************************/
/*                                                                 */
/*  compute_lexical_heads                                          */
/*                                                                 */
/*******************************************************************/

static void compute_lexical_heads( Parse &p, Parse &lp )

{
  LexInfo lexinfo( p.number_of_nodes() );

  Node root = Node(0);
  lexinfo.extract_heads( root );
  lexinfo.propagate_heads( p );

  vector<size_t> head;
  for( Index i=0; i<p.number_of_nodes(); i++ ) {
    lexinfo.heads_for( i, head );
    assert(head.size() > 0);
    first_lex_node.push_back((Index)lp.head.size());
    for( size_t k=0; k<head.size(); k++ )
      lp.head.push_back( (HeadIndex)head[k] );
  }
  lp.symbol.resize( lp.head.size() );
  lp.first_analysis.resize( lp.head.size()+1 );
  first_lex_node.push_back((Index)lp.head.size());
}


/*******************************************************************/
/*                                                                 */
/*  count_analyses                                                 */
/*                                                                 */
/*******************************************************************/

static void count_analyses( Node node, vector<Index> &next_analysis, 
			    vector<Index> &next_daughter, Parse &lp )
{
  AnalysisCount ac( node, lp );
		 
  for( Node::iterator it=node.begin(); it!=node.end(); ++it ) {
    Edge edge=*it;

    if (edge.is_terminal())
      ac.add( edge, 0, edge.head() );

    else if (edge.is_unary()) {
      Node d = *edge.begin();
      int n = d.number();
      for( Index i=first_lex_node[n]; i<first_lex_node[n+1]; i++ )
	ac.add( edge, 1, lp.head[i] );
    }
    else {
      Edge::iterator it=edge.begin();
      Node d1 = *it;
      Node d2 = *(++it);
      int n1 = d1.number();
      int n2 = d2.number();
      HeadDir dir = edge.binary_rule().headdir;
      for( Index i1=first_lex_node[n1]; i1<first_lex_node[n1+1]; i1++ )
	for( Index i2=first_lex_node[n2]; i2<first_lex_node[n2+1]; i2++ )
	  if (dir == left)
	    ac.add( edge, 2, lp.head[i1] );
	  else
	    ac.add( edge, 2, lp.head[i2] );
    }
  }
    
  // allocate space for the new lexicalized nodes

  // space for symbol, head and first_analysis was already 
  // allocated in Parser::compute_lexical_heads
    
  size_t fa = lp.rule_data.size();
  size_t N = fa + ac.total_number_of_analyses();
  lp.rule_data.resize( N );
  lp.first_daughter.resize( N+1 );
      
  size_t fd = lp.daughter.size();
  N = fd + ac.total_number_of_daughters();
  lp.daughter.resize( N );
      
  // insert information about first_analyses
      
  next_analysis.resize(ac.number_of_heads());
  next_daughter.resize(ac.number_of_heads());
  size_t n = first_lex_node[node.number()];
  for( size_t i=0; i<ac.number_of_heads(); i++, n++ ) {
    lp.symbol[n] = node.category();
    lp.first_analysis[n] = (Index)fa;
    lp.first_daughter[fa] = (Index)fd;
    next_analysis[i] = (Index)fa;
    next_daughter[i] = (Index)fd;
    fa += ac.number_of_analyses[i];
    fd += ac.number_of_daughters[i];
  }
  lp.first_analysis[n] = (Index)fa;
  lp.first_daughter[fa] = (Index)fd;
}


/*******************************************************************/
/*                                                                 */
/*  build_lex_parse                                                */
/*                                                                 */
/*******************************************************************/

static void build_lex_parse( Node node, Parse &lp )

{
  if (Visited[node.number()])
    return;
  Visited[node.number()] = true;

  vector<Index> next_analysis;
  vector<Index> next_daughter;

  count_analyses( node, next_analysis, next_daughter, lp );

  Index fln = first_lex_node[node.number()];

  // build the parse forest
  for( Node::iterator it=node.begin(); it!=node.end(); ++it ) {
    Edge edge=*it;
    
    if (edge.is_terminal()) {
      // store the new analysis
      int i = find_node_with_head(lp, fln, edge.head());
      lp.rule_data[next_analysis[i]] = edge.rule_data();
      lp.first_daughter[next_analysis[i]++] = next_daughter[i];
      break;
    }

    else if (edge.is_unary()) {
      Node d = *edge.begin();

      build_lex_parse( d, lp );

      // store the new analyses
      Index n = d.number();
      for( Index dn=first_lex_node[n]; dn<first_lex_node[n+1]; dn++ ) {
	int i = find_node_with_head(lp, fln, lp.head[dn]);
	lp.rule_data[next_analysis[i]] = edge.rule_data();
	lp.first_daughter[next_analysis[i]++] = next_daughter[i];
	lp.daughter[next_daughter[i]++] = dn;
      }
    }
     
    else {
      Edge::iterator it=edge.begin();
      Node d1 = *it;
      Node d2 = *(++it);
      HeadDir dir = edge.binary_rule().headdir;
      build_lex_parse( d1, lp );
      build_lex_parse( d2, lp );

      // store all possible analyses

      Index n1 = d1.number();
      Index n2 = d2.number();
      for( Index dn1=first_lex_node[n1]; dn1< first_lex_node[n1+1]; dn1++ )
	for( Index dn2=first_lex_node[n2]; dn2< first_lex_node[n2+1]; dn2++) {
	  int i;
	  if (dir == left)
	    i = find_node_with_head(lp, fln, lp.head[dn1]);
	  else
	    i = find_node_with_head(lp, fln, lp.head[dn2]);
	  lp.rule_data[next_analysis[i]] = edge.rule_data();
	  lp.first_daughter[next_analysis[i]++] = next_daughter[i];
	  lp.daughter[next_daughter[i]++] = dn1;
	  lp.daughter[next_daughter[i]++] = dn2;
	}
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  Parser::lexicalize_parse                                       */
/*                                                                 */
/*******************************************************************/

void Parser::lexicalize_parse()

{
  create_head_item_table();

  Visited = new bool[parse.number_of_nodes()];
  for( size_t i=0; i<parse.number_of_nodes(); i++ )
    Visited[i] = false;

  Parse lparse;
  compute_lexical_heads( parse, lparse );

  for( size_t i=0; i<parse.number_of_nodes(); i++ )
    Visited[i] = false;

  Node root = *begin();
  build_lex_parse( root, lparse );

  delete[] Visited;
  parse.number_of_roots = first_lex_node[1];
  vector<Index>().swap(first_lex_node);

  parse.symbol.swap(lparse.symbol); 
  parse.head.swap(lparse.head);
  parse.first_analysis.swap(lparse.first_analysis);
  parse.rule_data.swap(lparse.rule_data);
  parse.first_daughter.swap(lparse.first_daughter);
  parse.daughter.swap(lparse.daughter);
}
