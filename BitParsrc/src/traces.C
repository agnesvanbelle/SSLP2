
/*MA****************************************************************/
/*                                                                 */
/*     File: traces.C                                              */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jan  2 15:51:38 2003                              */
/* Modified: Mon Feb 23 11:55:48 2009 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/

#include "parser.h"


class Data {

public:
  unsigned short pos;
  SymNum symbol;
  Data( unsigned short p, SymNum s ) { pos = p; symbol = s; }
};

class TraceProbTab {
  
public:
  
private:
  struct eqf {
    bool operator()(const Data &d1, const Data &d2) const {
      return (d1.pos == d2.pos && d1.symbol == d2.symbol);
    }
  };
  struct hashf {
    size_t operator()(const Data &d) const { 
      return (d.symbol << 10) ^ d.pos;
    }
  };
  
  typedef hash_map<const Data, double, hashf, eqf> Table;

  Table tab;

public:
  typedef Table::iterator iterator;
  iterator begin() { return tab.begin(); };
  iterator end()   { return tab.end();   };
  size_t   size()  { return tab.size();  };

  double &operator[]( Data d ) {
    iterator it=tab.find(d);
    if (it == end())
      return tab.insert(Table::value_type(d, 0.0)).first->second;
    return it->second;
  };

  void clear() { 
    Table().swap(tab);  // clear and reduce capacity to 0
  };
};

static TraceProbTab TraceProb;
static int *EndPos;



/*******************************************************************/
/*                                                                 */
/*  quote                                                          */
/*                                                                 */
/*******************************************************************/

static char *quote( const char *string )

{
  int i,k;
  static char buffer[10000];

  for( i=k=0; k<10000; i++, k++) {
    if (!string[i])
      break;
    if  (string[i] == '"' || string[i] == '\\' || string[i] == ' ' || 
	 string[i] == '$' || string[i] == '^' || string[i] == '\'' ||
	 string[i] == '[' || string[i] == ']' || string[i] == '{' || 
	 string[i] == '}' || string[i] == '=' || string[i] == '<' || 
	 string[i] == '>' || string[i] == '#')
      buffer[k++] = '\\';
    buffer[k] = string[i];
  }
  buffer[k] = '\0';

  return &buffer[0];
}


/*******************************************************************/
/*                                                                 */
/*  Parser::add_trace_prob_edge                                    */
/*                                                                 */
/*******************************************************************/

int Parser::add_trace_prob_edge( Edge &edge, int abspos, int rulepos,
				 vector<Trace> &trace, int n, Prob p )
{
  for(Edge::iterator it=edge.begin(); it!=edge.end(); ++it) {
    Node node=*it;
    
    if (rulepos == trace[n].pos) {
      Data data((unsigned short)abspos, trace[n].sn);
      TraceProb[data] += p;
      n++;
    }
    if (node.is_aux())
      rulepos = add_trace_prob_edge(edge, abspos, rulepos, trace, n, p );
    else
      rulepos++;
    abspos = EndPos[node.number()];
  }
  return n;
}


/*******************************************************************/
/*                                                                 */
/*  Parser::add_trace_prob                                         */
/*                                                                 */
/*******************************************************************/

int Parser::add_trace_prob( Node node, int abspos )

{
  if (EndPos[node.number()] > -1)
    return EndPos[node.number()];
  EndPos[node.number()] = 0; // will later be updated

  // recursion
  for( Node::iterator it=node.begin(); it!=node.end(); ++it ) {
    Edge edge=*it;
    if (edge.is_terminal())
      EndPos[node.number()] = abspos + 1;
    else {
      int n = abspos;
      for(Edge::iterator it=edge.begin(); it!=edge.end(); ++it) {
	Node daughter = *it;
	n = add_trace_prob( daughter, n );
      }
      if (n > 0) // not a cyclic node
	EndPos[node.number()] = n;
    }
  }

  if (!node.is_aux()) {
    for( Node::iterator it=node.begin(); it!=node.end(); ++it ) {
      Edge e=*it;
      if (!e.is_terminal()) {
	RuleNumber rn = e.source_rule_number();
	vector<Trace> &trace = grammar.traces.get( rn );
	int n = add_trace_prob_edge( e, abspos, 0, trace, 0, e.freq());
	if (n < (int)trace.size()) {
	  assert(n == (int)trace.size()-1);
	  Data data((unsigned short)EndPos[node.number()], trace[n].sn);
	  TraceProb[data] += e.freq();
	}
      }
    }
  }
  return EndPos[node.number()];
}


/*******************************************************************/
/*                                                                 */
/*  Parser::print_trace_probs                                      */
/*                                                                 */
/*******************************************************************/

void Parser::print_trace_probs( FILE *file )

{
  if (parse.number_of_nodes() == 0) {
    failure_output( file );
    return;
  }

  estimate_freqs();

  EndPos = new int[parse.number_of_nodes()];
  for( size_t i=0; i<parse.number_of_nodes(); i++ )
    EndPos[i] = -1;
  for( iterator it=begin(); it!=end(); ++it ) {
    Node root = *it;
    add_trace_prob(root, 0);
  }
  delete[] EndPos;

  fprintf(file,"\nTrace probabilities\n");
  for(TraceProbTab::iterator it=TraceProb.begin(); it!=TraceProb.end(); ++it){
    Data dat=it->first;
    double prob=it->second;
    fprintf(file,"%s %d %.4f\n", 
	    quote(grammar.traces.symbol_name(dat.symbol)), dat.pos, prob);
  }

  TraceProb.clear();
}


/*******************************************************************/
/*                                                                 */
/*  Traces::print_trace                                            */
/*                                                                 */
/*******************************************************************/

void Traces::print_trace( int rn, size_t &tpos, size_t dpos, FILE *file )

{
  if (WithTraces && rn >= 0 && rn < (int)traces.size()) {
    vector<Trace> &trace=traces[rn];
    while (tpos < trace.size() && trace[tpos].pos == (int)dpos) {
      fprintf( file, " %s", quote(symbol_name(trace[tpos].sn)));
      tpos++;
    }
  }
}
