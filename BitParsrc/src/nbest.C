
/*******************************************************************/
/*                                                                 */
/*     File: nbest.C                                               */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Wed Jul 12 09:11:47 2006                              */
/* Modified: Thu Sep 13 14:11:36 2012 (schmid)                     */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*                                                                 */
/*******************************************************************/

#include <queue>
using std::priority_queue;

#include "quote.h"
#include "parser.h"

class NBestParse {
public:
  Prob prob;
  Node root;
  // The ncd array stores a sequence of indices.
  // Each index specifies the depth at which the next
  // switch to a less probable analysis of a node occurs
  vector<unsigned short> ncd;

  NBestParse() {}
  NBestParse( Prob p, Node r, vector<unsigned short> &pp ) 
    : prob(p), root(r), ncd(pp) {}

  bool operator<(const NBestParse &p) const {
    return prob < p.prob;
  }
};

struct cmp_prob {
  bool operator()( const Prob p1, const Prob p2 ) const {
    return p1 > p2;
  }
};

class BestParses {
  size_t nbest;

  // queue for storing the parses sorted by probability
  priority_queue<NBestParse> parses;

  // queue for storing the probabilities of the current n-best parses
  priority_queue<Prob, vector<Prob>, cmp_prob> probs;

public:
  bool add( Prob p, Node r, vector<unsigned short> &pp ) {
    if (probs.size() == nbest) {
      if (p <= probs.top())
	return false; // n better or equally good parses exist
      probs.pop(); // remove the worst parse
    }
    probs.push(p);
    parses.push(NBestParse( p, r, pp));
    return true;
  }

  bool empty() { return parses.empty(); }
  const NBestParse &top() { return parses.top(); }
  void pop() { parses.pop(); }

  BestParses( size_t n ) : nbest(n) {}
};


/*******************************************************************/
/*                                                                 */
/*  add_parses                                                     */
/*                                                                 */
/*******************************************************************/

static void add_parses( NBestParse &p, Node node, vector<unsigned short> &ncd, 
			size_t &index, unsigned short &depth, BestParses &bp )
{
  depth++;

  // create the sorted list of edges
  vector<Edge> edge;
  for( Node::iterator it=node.begin(); it!=node.end(); ++it )
    edge.push_back( *it );
  sort(edge.begin(), edge.end());

  if (index == ncd.size()) {   // end of position list reached?
    // add trees with the sub-optimal analyses of the current node
    size_t old_size = ncd.size();
    Prob pp = (p.prob == (Prob)0.0) ? (Prob)0.0 : p.prob / node.prob();
    for( size_t i=1; i<edge.size(); i++ ) {
      ncd.push_back( depth );
      if (!bp.add( pp * edge[i].prob(), p.root, ncd ))
	break;
    }
    ncd.resize( old_size );

    // add trees with sub-optimal analyses of the daughter nodes
    for( Edge::iterator it=edge[0].begin(); it!=edge[0].end(); ++it )
      add_parses( p, *it, ncd, index, depth, bp );
  }

  else {   // end of position list not reached yet
    // find the current edge
    size_t i;
    for( i=0; index<ncd.size() && ncd[index]==depth; i++ )
      index++;
    Edge &e = edge[i];
    for( Edge::iterator it=e.begin(); it!=e.end() && index<=ncd.size(); ++it )
      add_parses( p, *it, ncd, index, depth, bp );
  }
}


/*******************************************************************/
/*                                                                 */
/*  print_nbest_node                                               */
/*                                                                 */
/*******************************************************************/

static void print_nbest_node( Node node, vector<unsigned short> &ncd, 
			      size_t &index, unsigned short &depth, 
			      int rn, size_t &tpos, size_t &dpos, FILE *file)
{
  depth++;
  if (!node.is_aux())
    fprintf( file, "(%s ", quote(node.symbol_name()));

  // create the sorted list of edges
  vector<Edge> edge;
  for( Node::iterator it=node.begin(); it!=node.end(); ++it )
    edge.push_back( *it );
  sort(edge.begin(), edge.end());

  // find the current edge
  size_t i;
  for( i=0; index<ncd.size() && ncd[index]==depth; i++ )
    index++;
  Edge &e = edge[i];
  if (e.is_terminal())
    fputs( quote(e.word()), file );
  else {
    if (node.is_aux())
      for( Edge::iterator it=e.begin(); it!=e.end(); ++it )
	print_nbest_node( *it, ncd, index, depth, rn, tpos, dpos, file );
    else {
      size_t tp = 0;
      size_t dp = 0;
      int rn = e.source_rule_number();
      node.my_parser()->grammar.traces.print_trace( rn, tp, dp, file );
      for( Edge::iterator it=e.begin(); it!=e.end(); ++it )
	print_nbest_node( *it, ncd, index, depth, rn, tp, dp, file );
    }
  }

  if (!node.is_aux())
    fputc( ')', file );

  node.my_parser()->grammar.traces.print_trace( rn, tpos, ++dpos, file );
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_nbest_parses                                     */
/*                                                                 */
/*******************************************************************/

void Parser::print_nbest_parses( FILE *file )

{
  if (parse.number_of_nodes() == 0)
    failure_output( file );

  else {
    BestParses bp(NBest);
    vector<unsigned short> next_change_at_depth;

    // add the best parse tree for each top-most category
    for( iterator it=begin(); it!=end(); ++it ) {
      Node root = *it;
      bp.add( root.prob(), root, next_change_at_depth );
    }
    
    // add the next best parse trees
    for( size_t i=0; i<NBest && !bp.empty(); i++ ) {
      // Get the next parse from the queue of best (remaining) parses
      NBestParse p = bp.top();
      bp.pop();

      // print the parse
      if (ViterbiProbs)
	fprintf(file, "logvitprob=%g\n", p.prob.log_val());
      size_t index=0;
      unsigned short depth=0;
      size_t tp=0, dp=0;
      int rn=0;
      print_nbest_node( p.root, p.ncd, index, depth, rn, tp, dp, file);
      fputc( '\n', file );

      // generate derivative parses
      next_change_at_depth = p.ncd;
      index = 0;
      depth = 0;
      add_parses( p, p.root, next_change_at_depth, index, depth, bp);
    }
  }

  fputc( '\n', file );
  fflush(file);
}
