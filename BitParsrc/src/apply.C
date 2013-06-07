/*******************************************************************/
/*      File: /home/helmut/src/BitPar/apply.C                      */
/*    Author: Helmut Schmid                                        */
/*   Purpose: basic parse forest processing algorithm              */
/*   Created: Tue Dec 10 10:57:14 2002                             */
/*  Modified: Fri Jul  7 16:34:28 2006 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include "parser.h"

static bool *Visited;

static void (*NodePreF)( Node );
static void (*NodePostF)( Node );
static void (*AnaPreF)( Edge& );
static void (*AnaPostF)( Edge& );
static void (*EdgePreF)( Edge& );
static void (*EdgePostF)( Edge& );

static void node_apply( Node node );


/*******************************************************************/
/*                                                                 */
/*  edge_apply                                                     */
/*                                                                 */
/*******************************************************************/

static void edge_apply( Edge &edge )

{
  if (EdgePreF)
    EdgePreF(edge);

  Edge::iterator end=edge.end();
  for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
    Node node=*it;
    node_apply(node);
  }

  if (EdgePostF)
    EdgePostF(edge);
}


/*******************************************************************/
/*                                                                 */
/*  node_apply                                                     */
/*                                                                 */
/*******************************************************************/

static void node_apply( Node node )

{
  if (Visited[node.number()])
    return;

  Visited[node.number()] = true;
  bool not_aux = !node.is_aux();
  
  if (NodePreF)
    NodePreF(node);
  
  Node::iterator end = node.end();
  for( Node::iterator it=node.begin(); it!=end; ++it ) {
    Edge edge=*it;
    if (not_aux && AnaPreF)
      AnaPreF(edge);
    
    edge_apply(edge);
    
    if (not_aux && AnaPostF)
      AnaPostF(edge);
  }
  
  if (NodePostF)
    NodePostF(node);
}


/*******************************************************************/
/*                                                                 */
/*  Parser::apply                                                  */
/*                                                                 */
/*******************************************************************/

void Parser::apply( void (*npre)(Node), void (*npost)(Node), 
		    void (*epre)(Edge&), void (*epost)(Edge&), 
		    void (*apre)(Edge&), void (*apost)(Edge&), 
		    void (*rpre)(Node), void (*rpost)(Node) )
{
  NodePreF  = npre;
  NodePostF = npost;
  AnaPreF   = apre;
  AnaPostF  = apost;
  EdgePreF  = epre;
  EdgePostF = epost;

  Visited = new bool[parse.number_of_nodes()];
  for( size_t i=0; i<parse.number_of_nodes(); i++ )
    Visited[i] = false;

  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;
    if (rpre)
      rpre(root);

    node_apply(root);

    if (rpost)
      rpost(root);
  }

  delete[] Visited;
}
