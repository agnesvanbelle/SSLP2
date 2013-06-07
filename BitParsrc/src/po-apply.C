/*******************************************************************/
/*      File: /home/helmut/src/BitPar/po-apply.C                   */
/*    Author: Helmut Schmid                                        */
/*   Purpose: basic parse forest algorithm for the implementation  */
/*            of the outside algorithm and similar algorithms      */
/*   Created: Tue Dec 10 11:21:14 2002                             */
/*  Modified: Wed Jan 14 17:49:47 2009 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include "parser.h"


static void (*NodePreF)( Node );
static void (*NodePostF)( Node );
static void (*AnaPreF)( Edge& );
static void (*AnaPostF)( Edge& );
static void (*EdgePreF)( Edge& );
static void (*EdgePostF)( Edge& );

static void node_po_apply( Node node );

static bool *CurrentlyVisited;
static int *RefCount;


/*******************************************************************/
/*                                                                 */
/*  node_ref_count                                                 */
/*                                                                 */
/*******************************************************************/

static void node_ref_count( Node node )

{
  if (CurrentlyVisited[node.number()])
    return; // recursion

  if (++RefCount[node.number()] == 1) {
    CurrentlyVisited[node.number()] = true;
    Node::iterator end = node.end();
    for( Node::iterator it=node.begin(); it!=end; ++it ) {
      Edge edge=*it;
      Edge::iterator end = edge.end();
      for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
	Node daughter=*it;
	node_ref_count(daughter);
      }
    }
    CurrentlyVisited[node.number()] = false;
  }
}


/*******************************************************************/
/*                                                                 */
/*  edge_po_apply                                                  */
/*                                                                 */
/*******************************************************************/

static void edge_po_apply( Edge &edge )
  
{
  if (EdgePreF)
    EdgePreF(edge);
  
  Edge::iterator end = edge.end();
  for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
    Node node=*it;
    node_po_apply(node);
  }
  
  if (EdgePostF)
    EdgePostF(edge);
}


/*******************************************************************/
/*                                                                 */
/*  node_po_apply                                                  */
/*                                                                 */
/*******************************************************************/

static void node_po_apply( Node node )

{
  if (CurrentlyVisited[node.number()])
    return; // recursion

  if (RefCount[node.number()] > 0) {
    if (--RefCount[node.number()] == 0) {
      bool not_aux=!node.is_aux();
      if (NodePreF)
	NodePreF(node);
      
      CurrentlyVisited[node.number()] = true;
      Node::iterator end = node.end();
      for( Node::iterator it=node.begin(); it!=end; ++it ) {
	Edge edge=*it;
	if (not_aux && AnaPreF)
	  AnaPreF(edge);
	
	edge_po_apply(edge);
	
	if (not_aux && AnaPostF)
	  AnaPostF(edge);
      }
      CurrentlyVisited[node.number()] = false;
      
      if (NodePostF)
	NodePostF(node);
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  Parser::po_apply                                               */
/*                                                                 */
/*******************************************************************/

bool Parser::po_apply( void (*npre)(Node), void (*npost)(Node), 
		       void (*epre)(Edge&), void (*epost)(Edge&), 
		       void (*apre)(Edge&), void (*apost)(Edge&), 
		       void (*rpre)(Node), void (*rpost)(Node))
{
  NodePreF  = npre;
  NodePostF = npost;
  AnaPreF   = apre;
  AnaPostF  = apost;
  EdgePreF  = epre;
  EdgePostF = epost;

  CurrentlyVisited = new bool[parse.number_of_nodes()];
  RefCount = new int[parse.number_of_nodes()];
  for( size_t i=0; i<parse.number_of_nodes(); i++ ) {
    CurrentlyVisited[i] = false;
    RefCount[i] = false;
  }

  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;

    node_ref_count( root );
  }

  bool recursive=false;
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;

    if (rpre)
      rpre(root);

    node_po_apply(root);
    
    // Processing of recursive nodes
    bool repeat;
    do {
      repeat = false;
      for( size_t i=0; i<parse.number_of_nodes(); i++ )
	if (RefCount[i] > 0) {
	  Node node((Index)i);
	  node_po_apply(node);
	  recursive = true;
	  if (RefCount[i] > 0)
	    repeat = true;
	}
    } while (repeat);

    if (rpost)
      rpost(root);
  }

  delete[] RefCount;
  delete[] CurrentlyVisited;
  return recursive;
}
