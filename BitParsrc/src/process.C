/*******************************************************************/
/*      File: process.C                                            */
/*    Author: Helmut Schmid                                        */
/*   Purpose: selection of the most probable parse                 */
/*   Created: Tue Dec 10 10:57:14 2002                             */
/*  Modified: Fri Feb 25 13:45:03 2011 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include <iostream>
using std::cerr;

#include "parser.h"

static bool VitProb=false;
static bool Finished;


/*******************************************************************/
/*                                                                 */
/*  edge_func                                                      */
/*                                                                 */
/*******************************************************************/

static void edge_func( Edge &edge )

{
  Prob p = 1.0;
  Edge::iterator end = edge.end();
  for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
    Node node=*it;
    p *= node.prob();
  }
  edge.prob() = p;
}


/*******************************************************************/
/*                                                                 */
/*  ana_func                                                       */
/*                                                                 */
/*******************************************************************/

static void ana_func( Edge &edge )

{
  edge.prob() *= edge.rule_prob();
}


/*******************************************************************/
/*                                                                 */
/*  node_func                                                      */
/*                                                                 */
/*******************************************************************/

static void node_func( Node node )

{
  Prob p = 0.0;
  Node::iterator end = node.end();
  for( Node::iterator it=node.begin(); it!=end; ++it ) {
    Edge edge=*it;
    if (VitProb) {
      if (p <= edge.prob()) {
	p = edge.prob();
	if (p < edge.prob())
	  Finished = false;
      }
    }
    else // Inside algorithm
      p += edge.prob();
  }

  if (node.prob() < p * 0.999999999999) {
    node.prob() = p;
    Finished = false;
  }
}


/*******************************************************************/
/*                                                                 */
/*  Parser::viterbi                                                */
/*                                                                 */
/*******************************************************************/

void Parser::viterbi()

{
  VitProb = true;

  if (NodeProb.size() > 0) {
    vector<Prob>().swap(NodeProb); // clear and free the memory
    vector<Prob>().swap(EdgeProb); // clear and free the memory
  }
  NodeProb.resize(parse.number_of_nodes(), (Prob)0.0);
  EdgeProb.resize(parse.number_of_edges(), (Prob)0.0);

  int n=0;
  do {         // multiple passes required in order to approximate 
    n++;       // the Inside probabilities in cyclic grammars 
    Finished = true;
    apply(NULL, node_func, NULL, edge_func, NULL, ana_func);
  } while (!Finished);

  if (verbose)
    cerr << "\n" << n << " iterations to compute inside probabilities\n";
}


/*******************************************************************/
/*                                                                 */
/*  Parser::inside                                                 */
/*                                                                 */
/*******************************************************************/

void Parser::inside()

{
  VitProb = false;

  if (NodeProb.size() > 0) {
    vector<Prob>().swap(NodeProb); // clear and free the memory
    vector<Prob>().swap(EdgeProb); // clear and free the memory
  }
  NodeProb.resize(parse.number_of_nodes(), (Prob)0.0);
  EdgeProb.resize(parse.number_of_edges(), (Prob)0.0);

  int n=0;
  do {         // multiple passes required in order to approximate 
    n++;       // the Inside probabilities in cyclic grammars 
    Finished = true;
    apply(NULL, node_func, NULL, edge_func, NULL, ana_func);
  } while (!Finished);

  if (verbose)
    cerr << "\n" << n << " iterations to compute inside probabilities\n";
}
