
/*MA****************************************************************/
/*                                                                 */
/*     File: /home/users2/schmid/src/BitPar/estimate-freqs.C       */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Dec 19 13:37:45 2002                              */
/* Modified: Mon Feb 23 10:17:56 2009 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include "parser.h"

static bool Finished;
static vector<Prob> OldNodeFreq;


/*******************************************************************/
/*                                                                 */
/*  edge_func                                                      */
/*                                                                 */
/*******************************************************************/

static void edge_func( Edge &edge )

{
  Prob f=edge.freq();
  Edge::iterator end = edge.end();
  for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
    Node node=*it;
    node.freq() += f;
  }
}


/*******************************************************************/
/*                                                                 */
/*  node_func                                                      */
/*                                                                 */
/*******************************************************************/

static void node_func( Node node )

{
  Prob f=node.freq();
  if (OldNodeFreq[node.number()] < f * 0.999999999999) {
    Finished = false;
    OldNodeFreq[node.number()] = f;
  }
  Node::iterator end = node.end();
  f = f / node.prob();
  for( Node::iterator it=node.begin(); it!=end; ++it ) {
    Edge edge=*it;
    edge.freq() = f * edge.prob();
  }
  node.freq() = (Prob)0.0;
}


/*******************************************************************/
/*                                                                 */
/*  Parser::estimate_freqs                                         */
/*                                                                 */
/*******************************************************************/

void Parser::estimate_freqs()

{
  inside();

  if (NodeFreq.size() > 0) {
    vector<Prob>().swap(NodeFreq); // clear and free the memory
    vector<Prob>().swap(EdgeFreq); // clear and free the memory
  }
  NodeFreq.resize(parse.number_of_nodes(), (Prob)0.0);
  EdgeFreq.resize(parse.number_of_edges(), (Prob)0.0);
  OldNodeFreq.resize(parse.number_of_nodes(), (Prob)0.0);

  Prob root_prob_sum;
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root=*it;
    root_prob_sum += root.prob();
  }
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root=*it;
    OldNodeFreq[root.number()] = root.prob() / root_prob_sum;
  }

  bool recursive;
  int n=0;
  do {
    n++;
    Finished = true;
    for( iterator it=begin(); it!=end(); ++it ) {
      Node root=*it;
      root.freq() = OldNodeFreq[root.number()];
    }
    recursive = po_apply(node_func, NULL, edge_func, NULL);
  } while (recursive && !Finished);

  if (verbose && n>1)
    fprintf(stderr,"\n%d iterations for computation of estimated frequencies...", n);

  for( size_t i=0; i<parse.number_of_nodes(); i++ )
    NodeFreq[i] = OldNodeFreq[i];
  vector<Prob>().swap(OldNodeFreq); // clear and free the memory
}


/*******************************************************************/
/*                                                                 */
/*  ana_sum_func                                                   */
/*                                                                 */
/*******************************************************************/

static void ana_sum_func( Edge &edge )

{
  edge.incr_rule_freq();
}


/*******************************************************************/
/*                                                                 */
/*  Parser::train                                                  */
/*                                                                 */
/*******************************************************************/

void Parser::train()

{
  estimate_freqs();
  apply(NULL, NULL, NULL, NULL, NULL, ana_sum_func);
}
