/*******************************************************************/
/*      File: print-YAP.h                                          */
/*    Author: Helmut Schmid                                        */
/*   Purpose: print functions                                      */
/*   Created: Tue Dec 10 14:05:15 2002                             */
/*  Modified: Wed Apr 27 15:42:29 2011 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include "quote.h"

static int Current_ID;
static bool *Visited;
static int *NodeID;
static int  *StartPos;
static int  *EndPos;
static bool PrintHeads;
static bool PrintRP;
FILE *Outfile;


/*******************************************************************/
/*                                                                 */
/*  annotate_node                                                  */
/*                                                                 */
/*******************************************************************/

static int annotate_node( Node node, int pos )

{
  if (!Visited[node.number()]) {
    Visited[node.number()] = true;

    if (!node.is_aux())
      NodeID[node.number()] = Current_ID++;
    StartPos[node.number()] = pos;

    for( Node::iterator it=node.begin(); it!=node.end(); ++it ) {
      Edge edge=*it;
      if (edge.is_terminal())
	EndPos[node.number()] = StartPos[node.number()] + 1;
      else {
	pos = StartPos[node.number()];
	for( Edge::iterator it=edge.begin(); it!=edge.end(); ++it ) {
	  Node daughter=*it;
	  pos = annotate_node(daughter, pos);
	}
	EndPos[node.number()] = pos;
      }
    }
  }
  return EndPos[node.number()];
}


/*******************************************************************/
/*                                                                 */
/*  print_YAP_daughters                                            */
/*                                                                 */
/*******************************************************************/

static void print_YAP_daughters( Node node, vector<int> &currana, 
				 vector<int> &maxana, size_t &n, 
				 bool start=true )
{
  if (currana.size() == n) {
    currana.push_back(0);
    maxana.push_back((int)node.size());
  }
  Edge edge=node.edge(currana[n]);
  if (start && PrintRP)
    fprintf(Outfile, " %lg", edge.rule_prob());
  if (edge.is_terminal()) {
    if (PrintHeads)
      fputc(' ', Outfile);
    fprintf(Outfile, " \"%s\"", quote(edge.word()));
  }
  else {
    if (n == 0)
      fprintf(Outfile, " %d", edge.source_rule_number());
    for( Edge::iterator it=edge.begin(); it!=edge.end(); ++it ) {
      Node daughter=*it;
      if (daughter.is_aux())
	print_YAP_daughters(daughter, currana, maxana, ++n, false);
      else
	fprintf(Outfile, " %d", NodeID[daughter.number()]);
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  print_YAP_node                                                 */
/*                                                                 */
/*******************************************************************/

static void print_YAP_node( Node node )

{
  if (node.is_aux())
    return;

  fprintf(Outfile, "\n%s %d %d", node.symbol_name(), 
	  StartPos[node.number()], EndPos[node.number()]);
  if (PrintHeads)
    fprintf(Outfile," \"%s\" \"%s\"", node.head_string(), node.head_tag_name());
  fputc('\t',Outfile);

  vector<int> currana, maxana;
  if (Probs || Freqs) {
    vector<AnaProb> analyses;
    do {
      size_t n=0;
      Prob p = daughters_prob( node, currana, maxana, n, Outfile );
      if (Freqs)
	p *= node.freq() / node.prob();
      analyses.push_back(AnaProb(currana, p));
    } while (next_combination(currana, maxana));

    sort(analyses.begin(), analyses.end());
    
    for( size_t i=0; i<analyses.size(); i++ ) {
      if (Probs || Freqs)
	fprintf(Outfile," %g", (double)analyses[i].prob);
      size_t n=0;
      print_YAP_daughters(node, analyses[i].ana, maxana, n);
      fputs(" %", Outfile);
    }
  }
  else {
    do {
      size_t n=0;
      print_YAP_daughters( node, currana, maxana, n );
      fputs(" %", Outfile);
    } while (next_combination(currana, maxana));
  }
  fputc('%', Outfile);
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_YAP_parse                                        */
/*                                                                 */
/*******************************************************************/

void Parser::print_YAP_parse( FILE *file )

{
  Probs = ViterbiProbs | InsideProbs;
  Freqs = EstimatedFreqs;
  PrintHeads = Lexicalized;
  PrintRP = PrintRuleProbs;
  if (parse.number_of_nodes() == 0) {
    failure_output( file );
    return;
  }

  Visited = new bool[parse.number_of_nodes()];
  for( size_t i=0; i<parse.number_of_nodes(); i++ )
    Visited[i] = false;
  Current_ID = 0;
  NodeID   = new int[parse.number_of_nodes()];
  StartPos = new int[parse.number_of_nodes()];
  EndPos   = new int[parse.number_of_nodes()];

  Outfile = file;
  if (PrintHeads)
    fputc('\n',Outfile);
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;
    annotate_node(root, 0);
    if (PrintHeads)
      fprintf(Outfile, "%d ",NodeID[root.number()]);
  }
  delete[] Visited;
  if (PrintHeads)
    fputs("%%",Outfile);

  apply(print_YAP_node, NULL, NULL, NULL, NULL, NULL);
  fputs("%\n",file);

  delete[] NodeID;
  delete[] StartPos;
  delete[] EndPos;
}
