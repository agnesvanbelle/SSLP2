/*******************************************************************/
/*      File: /home/helmut/src/BitPar/print.h                      */
/*    Author: Helmut Schmid                                        */
/*   Purpose: print functions                                      */
/*   Created: Tue Dec 10 14:05:15 2002                             */
/*  Modified: Mon Mar 28 13:34:30 2011 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include "quote.h"

static bool Freqs, Probs;
static int Current_Tag=0;

class AnaProb {

 public:
  vector<int> ana;
  Prob prob;

  AnaProb( vector<int> a, Prob p ) : ana(a) { prob = p; };
    bool operator<(const AnaProb &a) const { return prob > a.prob; };
};



/*******************************************************************/
/*                                                                 */
/*  next_combination                                               */
/*                                                                 */
/*******************************************************************/

static bool next_combination( vector<int> &currana, vector<int> &maxana )

{
  assert(currana.size() > 0);
  while (++currana.back() == maxana.back()) {
    currana.pop_back();
    maxana.pop_back();
    if (currana.size() == 0)
      return false;
  }
  return true;
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_daughters                                        */
/*                                                                 */
/*******************************************************************/

void Parser::print_daughters( Node node, vector<int> &currana, 
			      vector<int> &maxana, size_t &n, 
			      int rn, size_t &tpos, size_t &dpos, 
			      vector<int> &Tag, FILE *file )
{
  if (currana.size() == n) {
    currana.push_back(0);
    maxana.push_back((int)node.size());
  }

  Edge edge=node.edge(currana[n]);
  if (edge.is_terminal()) {
    fprintf(file, " %s", quote(edge.word()));
    grammar.traces.print_trace( rn, tpos, ++dpos, file );
  }
  else {
    if (n == 0) {
      rn = edge.source_rule_number();
      if (PrintRuleNumbers && rn >= 0)
	fprintf(file, " %lu", (unsigned long)rn);
      grammar.traces.print_trace( rn, tpos, dpos, file );
    }
    
    Edge::iterator end = edge.end();
    for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
      Node daughter=*it;
      if (daughter.is_aux())
	print_daughters(daughter, currana, maxana, ++n,rn,tpos,dpos,Tag,file);
      else {
	print_node(daughter, Tag, file);
	grammar.traces.print_trace( rn, tpos, ++dpos, file );
      }
    }
  }
}


/*******************************************************************/
/*                                                                 */
/*  daughters_prob                                                 */
/*                                                                 */
/*******************************************************************/

static Prob daughters_prob( Node node, vector<int> &currana, 
			    vector<int> &maxana, size_t &n, FILE *file )
{
  if (currana.size() == n) {
    currana.push_back(0);
    maxana.push_back((int)node.size());
  }
  Edge edge=node.edge(currana[n]);
  if (edge.is_terminal())
    return edge.prob();

  Prob p = edge.rule_prob();

  Edge::iterator end = edge.end();
  for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
    Node daughter=*it;
    if (daughter.is_aux())
      p *= daughters_prob(daughter, currana, maxana, ++n, file);
    else
      p *= daughter.prob();
  }
  return p;
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_node                                             */
/*                                                                 */
/*******************************************************************/

void Parser::print_node( Node node, vector<int> &Tag, FILE *file )

{
  int &tag=Tag[node.number()];
  if (tag > 0) {
    // node has been printed previously
    fprintf(file," #%d", tag);
    return;
  } 
  else if (tag < -1) {
    // first visit of a reentrant node
    if (++Current_Tag == 0)
      fprintf(stderr,"Warning: too many different reentrant nodes in output!\n");
    tag = Current_Tag;
    fprintf(file," #%d=", tag);
  }
    
  vector<int> currana, maxana;
  bool ambig = node.is_ambiguous();
  if (ambig)
    fputc('{', file);

  if (Probs || Freqs) {
    vector<AnaProb> analyses;
    do {
      size_t n=0;
      Prob p = daughters_prob( node, currana, maxana, n, file );
      if (Freqs)
	p *= node.freq() / node.prob();
      analyses.push_back(AnaProb(currana, p));
    } while (next_combination(currana, maxana));

    sort(analyses.begin(), analyses.end());
    
    for( size_t i=0; i<analyses.size(); i++ ) {
      if (i > 0)
	fputs("#i",file);
      fprintf(file,"(%s", quote(node.symbol_name()));
      fprintf(file,"=#i[");
      if (Probs || Freqs) {
	fprintf(file,"P=%g", (double)analyses[i].prob);
	if (Lexicalized)
	  fprintf(file," ");
      }
      if (Lexicalized) {
	fprintf(file,"H=%s", quote(node.head_string()));
	fprintf(file," T=%s", quote(node.head_tag_name()));
      }
      fprintf(file,"]");

      size_t n=0, tp=0, dp=0;
      print_daughters(node, analyses[i].ana, maxana, n, 0, tp, dp, Tag, file);
      fputc(')',file);
    }
  }
  else {
    for(;;) {
      fprintf(file,"(%s", quote(node.symbol_name()));
      if (Lexicalized) {
	fprintf(file," #i=[H=%s", quote(node.head_string()));
	fprintf(file," T=%s]", quote(node.head_tag_name()));
      }
      size_t n=0, tp=0, dp=0;
      print_daughters( node, currana, maxana, n, 0, tp, dp, Tag, file );
      fputc(')',file);
      if (!next_combination(currana, maxana))
	break;
      fputs("#i",file);
    }
  }

  if (ambig)
    fputc('}', file);
}



/*******************************************************************/
/*                                                                 */
/*  Parser::mark_daughters                                         */
/*                                                                 */
/*******************************************************************/

void Parser::mark_daughters( Node node, vector<int> &currana, 
			     vector<int> &maxana, size_t &n, vector<int> &Tag)
{
  if (currana.size() == n) {
    currana.push_back(0);
    maxana.push_back((int)node.size());
  }

  Edge edge=node.edge(currana[n]);
  Edge::iterator end = edge.end();
  for( Edge::iterator it=edge.begin(); it!=end; ++it ) {
    Node daughter=*it;
    if (daughter.is_aux())
      mark_daughters(daughter, currana, maxana, ++n, Tag);
    else
      mark_node(daughter, Tag);
  }
}


/*******************************************************************/
/*                                                                 */
/*  Parser::mark_node                                              */
/*                                                                 */
/*******************************************************************/

void Parser::mark_node( Node node, vector<int> &Tag )

{
  if (--Tag[node.number()] < -1)
    return;
    
  vector<int> currana, maxana;
  do {
    size_t n=0;
    mark_daughters( node, currana, maxana, n, Tag );
  } while (next_combination(currana, maxana));
}


/*******************************************************************/
/*                                                                 */
/*  most_probable_edge                                             */
/*                                                                 */
/*******************************************************************/

Edge most_probable_edge( Node node )

{
  Node::iterator it=node.begin();
  Edge best_edge = *it;
  Prob best_prob = best_edge.prob();
  for( ++it; it!=node.end(); ++it ) {
    Edge edge=*it;
    Prob p = edge.prob();
    if (best_prob < p) {
      best_prob = p;
      best_edge = edge;
    }
  }
  return best_edge;
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_parse                                            */
/*                                                                 */
/*******************************************************************/

void Parser::print_parse( FILE *file )

{
  if (parse.number_of_nodes() == 0) {
    failure_output( file );
    return;
  }

  Probs = ViterbiProbs | InsideProbs;
  Freqs = EstimatedFreqs;
  
  vector<int> Tag(parse.number_of_nodes(), 0);
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;
    mark_node( root, Tag );
  }
  if (parse.number_of_roots > 1)
    fputc('{', file);
  if (Probs || Freqs) {
    vector<Node> sorted_root;
    
    for( iterator it=begin(); it!=end(); ++it )
	sorted_root.push_back(*it);
    sort(sorted_root.begin(), sorted_root.end());
    
    for( size_t i=0; i<sorted_root.size(); i++ ) {
      if (i > 0)
	fputs("#i",file);
      print_node(sorted_root[i], Tag, file); 
    }
  }
  else {
    for( iterator it=begin(); it!=end(); ++it ) {
      Node root = *it;
      if (it != begin())
	fputs("#i",file);
      print_node(root, Tag, file); 
    }
  }
  if (parse.number_of_roots > 1)
    fputc('}', file);

  fputc('\n',file);
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_best_node                                        */
/*                                                                 */
/*******************************************************************/

void Parser::print_best_node( Node node, int rn, size_t &tpos, size_t &dpos, 
			      FILE *file )
{
  // find the best analysis
  Edge best_edge = most_probable_edge( node );

  // print auxiliary nodes
  if (node.is_aux()) {
    for( Edge::iterator it=best_edge.begin(); it!=best_edge.end(); ++it ) {
      Node daughter=*it;
      print_best_node(daughter, rn, tpos, dpos, file );
    }
    return;
  }

  fprintf(file,"(%s ", quote(node.symbol_name()));
  if (EstimatedFreqs)
    fprintf(file, "=#i[F=%g] ", (double)best_edge.freq());

  // print terminal nodes
  if (best_edge.is_terminal())
    fputs(quote(best_edge.word()), file);

  // print subtrees
  else {
    size_t tp = 0;
    size_t dp = 0;
    int rn = best_edge.source_rule_number();
    // print initial traces
    grammar.traces.print_trace( rn, tp, dp, file );
    
    for( Edge::iterator it=best_edge.begin(); it!=best_edge.end(); ++it ) {
      Node daughter=*it;
      print_best_node(daughter, rn, tp, dp, file );
    }
  }
  fputc(')',file);
  grammar.traces.print_trace( rn, tpos, ++dpos, file );
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_best_parse                                       */
/*                                                                 */
/*******************************************************************/

void Parser::print_best_parse( FILE *file )

{
  if (parse.number_of_nodes() == 0) {
    failure_output( file );
    return;
  }

  Prob maxprob;
  Node best_root;
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;
    if (maxprob < root.prob()) {
      maxprob = root.prob();
      best_root = root;
    }
  }
  if (ViterbiProbs)
    fprintf(file, "logvitprob=%g\n", best_root.prob().log_val());
    
  size_t tp = 0;
  size_t dp = 0;
  print_best_node(best_root, -1, tp, dp, file );
  fputc('\n',file);
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_parse_tables                                     */
/*                                                                 */
/*******************************************************************/

void Parser::print_parse_tables( Parse &p )

{
  if (p.head_item.size() > 0) {
    fprintf(stderr,"\nindex\tsymbol\thead\tfirst_analysis\n");
    for( unsigned i=0; i<p.symbol.size(); i++ )
      fprintf(stderr,"%u\t%s\t%s\t%u\n", i, nfg.symbol_name(p.symbol[i]), 
	      p.head_item[p.head[i]].lemma,
	      p.first_analysis[i]);
  }
  else {
    fprintf(stderr,"\nindex\tsymbol\tfirst_analysis\n");
    for( unsigned i=0; i<p.symbol.size(); i++ )
      fprintf(stderr,"%u\t%s\t%u\n", i, nfg.symbol_name(p.symbol[i]), 
	      p.first_analysis[i]);
  }
  fprintf(stderr,"\nindex\trule_data\tfirst_daughter\n");
  for( unsigned i=0; i<p.rule_data.size(); i++ )
    fprintf(stderr,"%u\t%ld\t%ld\n", i, (long)p.rule_data[i], 
	    (long)p.first_daughter[i]);
  fprintf(stderr,"\nindex\tdaughter\n");
  for( unsigned i=0; i<p.daughter.size(); i++ )
    fprintf(stderr,"%u\t%ld\n", i, (long)p.daughter[i]);
  fputc('\n',stderr);
}
