import sys
from nltk.tree import *
from nltk.draw import tree
from  nltk import treetransforms
from collections import defaultdict
import re


normal_brackets = '[]'
inv_brackets = '<>'


def make_itg():
	pass
# put the initial tree nodes in a CYK-like chart
def make_syntax_chart(sentence_tree,  syntax_chart, col=0, row=None) :

  if row == None :
    row = len(sentence_tree.leaves())

  # if is not a leaf
  if isinstance(sentence_tree, tree.Tree):
    #print "%s %d %d" % (sentence_tree.node, col, row-1)

    # add to syntax chart
    if (col, row-1) in syntax_chart :
      syntax_chart[(col, row-1)] = sentence_tree.node + ":" + syntax_chart[(col, row-1)]
    else:
      syntax_chart[(col, row-1)] =  sentence_tree.node

    # add each kid (a node can have 1 or 2 kids) recursively
    lengthkids = 0
    for i in range (0, len(sentence_tree)):

      lengthkidsprev = lengthkids

      if isinstance(sentence_tree[i], tree.Tree):
        lengthkids += len(sentence_tree[i].leaves())
        length = len(sentence_tree[i].leaves())
      else:
        lengthkids += 1
        length = 1

      make_syntax_chart(sentence_tree[i], syntax_chart, col+lengthkidsprev, length+col+lengthkidsprev)
      #print "%s  %s  %d" % (sentence_tree[i], sentence_tree[i].node ,len(sentence_tree[i]))



def printdict(d) :
  for key, value in d.iteritems():
    print "%s --> %s" % (key, value)

# checks if a phrase is also continuous in the reordered format
# span should be a tuple where span[1] >= span[0]
def valid_phrase(span, reorderings_list):
  len_phrase = max(reorderings_list[span[0]:span[1]+1]) - min(reorderings_list[span[0]:span[1]+1])
  return len_phrase == span[1]-span[0]

def make_parse_chart(sentence_tree, reorderings_list) :

  #TODO: backpinters in parse chart to cell + to rule in cell

  # map: each key had one value
  syntax_chart = dict()
  make_syntax_chart(sentence_tree, syntax_chart)

  printdict(syntax_chart)

  len_sentence = len(sentence_tree.leaves())
  
  print "len_sentence: %s" % len_sentence
  # multimap by virtue of using lists as values
  parse_chart = defaultdict(list)

#  for key, value in syntax_chart.iteritems():      
 #   parse_chart[key].append(syntax_chart.get(key))
  for i in range(0, len_sentence):
    parse_chart[(i,i)] = [(syntax_chart[(i,i)],( 0, 0, 0))]
  print "parse chart:"
  printdict(parse_chart)

  for span_size in range(0, len_sentence) :
    for span_startpos in range (0, len_sentence-span_size):
      span_endpos = span_startpos + span_size
      span = (span_startpos, span_endpos)
      print " ==> span_size: %d \n span: %s " % (span_size, span)
      if valid_phrase(span, reorderings_list):
        print "span %s is a valid phrase " % (span,)

        for split_startpos in range (0, span_size):
          split_left = (span_startpos , span_startpos+ split_startpos)
          split_right = (span_startpos+split_startpos+1 , span_endpos)
          print "l: %s , r: %s " % (split_left, split_right)

          parent_extension = ""
          if is_inverted(split_left, split_right, reorderings_list):
            parent_extension = "_INV"

          leftkids = parse_chart[split_left]
          rightkids = parse_chart[split_right]

          #print "leftkids in %s: %s" % (split_left, leftkids)
          #print "rightkids in %s: %s" % (split_right, rightkids)
          
          for l in range(0, len(leftkids)):
            for r in range(0, len(rightkids)):
              print "leftkid: %s, rightkid: %s \n" % (leftkids[l], rightkids[r])
              parent_rule = get_parent_syntax_chart(syntax_chart,parse_chart, len_sentence, span, leftkids[l], rightkids[r])
              
              if not not parent_rule:
                print "parent_rule: %s" % parent_rule
                #if parent_rule[0] not in parse_chart[span]:
		parse_chart[span].extend( [(parent_rule[0] +  parent_extension, (split_right[0], l, r))])
		print "added %s at place %s" % (parent_rule, span)
		print "parse chart there now %s\n" % parse_chart[span]
              

      else :
        print "span %s is not a valid phrase " % (span,)
  
  printdict(parse_chart)
  return parse_chart

def get_parent_syntax_chart(syntax_chart, parse_chart,  len_sentence, parent_span, leftkid, rightkid):

  max_combined_rules = 1
  
  #if not not parse_chart[parent_span]:
  if not not syntax_chart.get(parent_span, ""):
    return [syntax_chart[parent_span]]
  else :
    # check for rule
    regex = "r[\\/+]+"

    #filter(lambda l:  l and len(l[0])<2, map(lambda a : re.findall(r"[\\/+]+",  a), s))
    if not too_many_combinations(leftkid[0], rightkid[0], max_combined_rules):
    #if not any (re.search(regex,  kid) for kid in [leftkid,rightkid]):
      return ["(" + leftkid[0] + "+" + rightkid[0] + ")"]
      
    else :
      # missing right part
      row = parent_span[1]
      while  row < len_sentence-1 :
        row += 1
        print "checking right missing for %s" % ((parent_span[0], row),)
        left_super = syntax_chart[(parent_span[0], row)]
        print "left_super: %s" % left_super
        if not not left_super: # if not empty cell          
          right_missing_row = row 
          right_missing_col = parent_span[1]+1
          right_missing = parse_chart[(right_missing_col, right_missing_row)]
          print "right_missing: %s at %s" % (right_missing, (right_missing_col, right_missing_row))
	  
	  result = []
	  for r in range(0, len(right_missing)):
	    if not too_many_combinations(left_super[0][0], right_missing[r][0], max_combined_rules):
	      result.extend( ["(" + left_super[0][0] + "/" + right_missing[r][0] + ")"])
	  return result
    
                      
      #  missing left part
      col = parent_span[0]
      while col > 0 :
        col -= 1
        print "checking left missing for %s" % ((col, parent_span[1]),)
        right_super = syntax_chart[(col, parent_span[1])]
        print "right_super: %s" % right_super
        if not not right_super: # if not empty cell          
          left_missing_row = parent_span[0]-1 
          left_missing_col = col
          left_missing = parse_chart[(left_missing_col, left_missing_row)]
          print "left_missing: %s at %s" % (left_missing, (left_missing_col, left_missing_row))
          
	  result = []
	  for l in range(0, len(left_missing)):
	    if not too_many_combinations(right_super[0][0], left_missing[l][0], max_combined_rules):
	      result.extend( ["(" + right_super[0][0] + "\\" + left_missing[l][0] + ")"])
	  return result
              
  return ""


def get_rules_from_parse_chart(parse_chart, sentence_tree):

  sentence_list = sentence_tree.leaves()
  len_sentence = len(sentence_list)
  binary_rules = [] 
  unary_rules = []

  for span_size in range(len_sentence-1, -1, -1):
	  for span_startpos in range(0, len_sentence-span_size):
		  span_endpos = span_startpos+span_size
		  span = (span_startpos, span_endpos)
		  print " ==> span_size: %d \n span: %s " % (span_size, span)
				  
		  parents = parse_chart[(span)]
		  for p in range(0, len(parents)):
		    parent = parents[p]
		    LHS = parent[0]
		    if parent[1][0]-1 >= 0: 
		      RHS1_loc = (span_startpos, parent[1][0]-1)
		      RHS2_loc = (parent[1][0], span_endpos)
		      RHS1 = parse_chart[RHS1_loc][parent[1][1]][0]
		      RHS2 = parse_chart[RHS2_loc][parent[1][2]][0]
		      binary_rules.append((LHS,(RHS1,RHS2)))
		      print (LHS,(RHS1,RHS2))
		    else:
		      unary_rules.append((LHS,sentence_list[span_startpos]))
		      print (LHS,sentence_list[span_startpos])
		      
#	for i in range(0, len(binary_rules)):
#	  print binary_rules[i]

  return [binary_rules,unary_rules]

def too_many_combinations(leftpart, rightpart, max_combined_rules):
  return len(filter(lambda l:  not l or len(l[0])<max_combined_rules, map(lambda a : re.findall(r"[\\/+]+",  a), [leftpart,rightpart]))) < 2
  
  
# a syntax label should be inverted if the spand_endpos of its left child
# is larger than the span_startpos of its right child
def is_inverted(leftspan, rightspan, reorderings_list):
  return reorderings_list[leftspan[1]] > reorderings_list[rightspan[0]]

def test() :
  sentence_tree = Tree('(S (NP (N man)) (VP (V bites) (NP (N dog))))')
  reorderings_list = [2, 1, 0]
   
  #sentence_tree = Tree('(S (ADVP (RB next)) (, ,) (NP (PRP we)) (VP (VBP have) (NP (NP (NNS flags)) (PP (IN of) (NP (NN convenience))))) (. .)) ')
  #reorderings_list = [0, 1, 2, 3, 4, 5, 6, 7]

  
  #sentence_tree = Tree('(A (G (H h) (I i) (J j)) (B (C (D d) (E e)) (F f)))')
  #reorderings_list = [0,1,2,3,4,5]
  
  
  #sentence_tree = Tree('(S (A a) (B b ) (C c) (D d))')
  #reorderings_list = [0,1,2,3]
  
  #treetransforms.chomsky_normal_form(sentence_tree, factor='right', horzMarkov=None, vertMarkov=0, childChar='|', parentChar='^')

 
  sentence_list = sentence_tree.leaves()

  #print list(enumerate(sentence_tree))

  #print sentence_tree[0]



  pc = make_parse_chart(sentence_tree, reorderings_list)
	
  [binary_rules, unary_rules] = get_rules_from_parse_chart(pc, sentence_tree)
  #sentence_tree.draw()


if __name__ == '__main__':
  test()
