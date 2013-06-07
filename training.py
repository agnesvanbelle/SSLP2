# tested with Python 2.7.3 and nltk 2.0.4 
# on deze.science.uva.nl (Linux 2.6.32 x86_64 (gcc version 4.4.6 (Red Hat 4.4.6-4))
# e.g.  python2.7 training.py -a data/train/europarl-v7*align* -p data/train/*parse*

import sys, os
from nltk.tree import *
from nltk.draw import tree
from  nltk import treetransforms
from collections import defaultdict
import re
import subprocess
import argparse

# maximum length of the sentences we're going to train on
max_sentence_len = 12

inv_suffix = "<"

filename_alignments = "data/train/europarl-v7.nl-en.align.en-en2"
filename_sentence_trees = "data/train/europarl-v7.nl-en.en.parse"

filename_temp_reorderings = "reorderings_permutations.dat"

filename_bitpar_grammar = "bitpar_grammar" + str(max_sentence_len) + ".dat"
filename_bitpar_lexicon = "bitpar_lexicon" + str(max_sentence_len) + ".dat"

## important:
# turn 'debug' on to see "print" messages
# if set to False, the only print messages that will be displayed
# are those which have 'output_symbol' as first character of the message
debug = False
output_symbol = ">"



# for compatibility with bitpar,
# the parser that we used for testing
# see http://www.cis.uni-muenchen.de/~schmid/tools/BitPar/
bitpar_top = 'TOP'
bitpar_top_child = 'S'
bitpar_top_prob = 1

#override sys.stdout (see comments about 'debug' global)
class writer :
  def __init__(self, *writers) :
    self.writers = writers

  def write(self, text) :

    if debug or text[0] == output_symbol:
      for w in self.writers :
        w.write(text)
sys.stdout = writer(sys.stdout)

# put the initial tree nodes in a CYK-like chart
# note that the initial tree is not necessarily in CNF (binary)
# however we will construct a binary ITG grammar from it later in make_parse_chart(..)
def make_syntax_chart(sentence_tree,  syntax_chart, col=0, row=None) :
  if row == None :
    row = len(sentence_tree.leaves())

  # if is not a leaf
  if isinstance(sentence_tree, tree.Tree):
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

def printdict(d) :
  for key, value in d.iteritems():
    print "%s --> %s" % (key, value)

# checks if a phrase is also continuous in the reordered format
# span should be a tuple where span[1] >= span[0]
def valid_phrase(span, reorderings_list):
  len_phrase = max(reorderings_list[span[0]:span[1]+1]) - min(reorderings_list[span[0]:span[1]+1])
  return len_phrase == span[1]-span[0]


# uses a cyk-like algorithm to construct a CYK-like chart to hold the ITG parses of the sentence 
# only parses phrase pairs considered valid given the reorderings!
# input:
#   sentence_tree: nltk.Tree
#   reorderings_list: list of integers denoting the reordering permutation, e.g. [0,3,2,1]
def make_parse_chart(sentence_tree, reorderings_list) :
  # map: each key has one value
  syntax_chart = defaultdict(lambda:"")
  make_syntax_chart(sentence_tree, syntax_chart)

  printdict(syntax_chart)

  len_sentence = len(sentence_tree.leaves())

  print "len_sentence: %s" % len_sentence
  
  # multimap by virtue of using lists as values
  parse_chart = defaultdict(list)

  for i in range(0, len_sentence):
    parse_chart[(i,i)] = [(syntax_chart[(i,i)],( 0, 0, 0))]

  print "parse chart:"
  printdict(parse_chart)

  for span_size in range(1, len_sentence) :
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

          suffix = ""
          if is_inverted(split_left, split_right, reorderings_list):
            suffix = inv_suffix

          leftkids = parse_chart[split_left]
          rightkids = parse_chart[split_right]
          
          for l in range(0, len(leftkids)):
            for r in range(0, len(rightkids)):
              print "leftkid: %s, rightkid: %s \n" % (leftkids[l], rightkids[r])
              parent_rule = get_parent_syntax_chart(syntax_chart,parse_chart, len_sentence, span, leftkids[l], rightkids[r])

              if not not parent_rule:
                print "parent_rule: %s" % parent_rule
                parse_chart[span].extend( [(parent_rule[0] + suffix, (split_right[0], l, r))])
                print "added %s at place %s" % (parent_rule, span)
                print "parse chart there now %s\n" % parse_chart[span]

      else :
        print "span %s is NOT a valid phrase " % (span,)

  print "parse chart:"
  printdict(parse_chart)
  return parse_chart


# get the parent for a left kid or right kid from the parse chart
# first tries to find one from the original syntax chart
# if not found, constructs one based on +, / or \ rules (SAMT-rules)
# if that is impossible due to limitations on multiple +, / and \ operators in one rule,
# returns the empty string
#
# note: what is returned here, is later put in the parse_chart in make_parse_chart(..)
def get_parent_syntax_chart(syntax_chart, parse_chart,  len_sentence, parent_span, leftkid, rightkid):

  #if you set this value higher the result is more +-rules since those rules have
  # preference over / or \-rules
  max_combined_rules = 1

  if not not syntax_chart.get(parent_span, ""):
    return [syntax_chart[parent_span]]
  else :
    # check for rule
    regex = "r[\\/+]+"

    if not too_many_combinations(leftkid[0], rightkid[0], max_combined_rules):
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
            if not too_many_combinations(left_super, right_missing[r][0], max_combined_rules):
              result.extend( ["(" + left_super + "/" + right_missing[r][0] + ")"])
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
            if not too_many_combinations(right_super, left_missing[l][0], max_combined_rules):
              result.extend( ["(" + right_super + "\\" + left_missing[l][0] + ")"])
          return result

  return ""

# extract the (SAMT) labels from the parse chart
def get_rules_from_parse_chart(parse_chart, sentence_tree):

  sentence_list = sentence_tree.leaves()
  len_sentence = len(sentence_list)
  grammar_rules = []
  lexical_rules = []

  for span_size in range(len_sentence-1, -1, -1):
    for span_startpos in range(0, len_sentence-span_size):
      span_endpos = span_startpos+span_size
      span = (span_startpos, span_endpos)
      print " ==> span_size: %d \n span: %s " % (span_size, span)

      parents = parse_chart[(span)]
      for p in range(0, len(parents)):
        parent = parents[p]
        LHS = parent[0]

        if parent[1][0]-1 >= 0: #binary rule

          RHS1_loc = (span_startpos, parent[1][0]-1)
          RHS2_loc = (parent[1][0], span_endpos)
          RHS1 = parse_chart[RHS1_loc][parent[1][1]][0]
          RHS2 = parse_chart[RHS2_loc][parent[1][2]][0]
          grammar_rules.append((LHS,(RHS1,RHS2)))
          print  (LHS,(RHS1,RHS2))

        else: #unary rule
          lexical_rules.append((LHS,sentence_list[span_startpos]))
          print (LHS,sentence_list[span_startpos])


  return [grammar_rules,lexical_rules]

# checks if the presence of a +, / or \ symbol is beneath 'max_combined_rules' times
def too_many_combinations(leftpart, rightpart, max_combined_rules):
  return len(filter(lambda l:  not l or len(l[0])<max_combined_rules, map(lambda a : re.findall(r"[\\/+]+",  a), [leftpart,rightpart]))) < 2


# checks if the left and right spand should be inverted
# a syntax label should be inverted if the spand_endpos of its left child
# is larger than the span_startpos of its right child
def is_inverted(leftspan, rightspan, reorderings_list):
  return reorderings_list[leftspan[1]] > reorderings_list[rightspan[0]]


#open the input files
def open_train_files(fn_align_perms, fn_sentence_trees):
  align_perms = 0
  sentence_trees = 0
  try:
    align_perms = open(fn_align_perms)
    sentence_trees  = open(fn_sentence_trees)
  except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
  except:
    print "Unexpected error:", sys.exc_info()[0]
    raise

  return [align_perms, sentence_trees]

#get the next line of each file as a list of two elements: 
#the reorderings-permutations list and the sentence tree as a nltk.Tree object
# returns list of two empty lists if it has reached EOF
def load_next_line( align_perms, sentence_trees):
  assert(align_perms and sentence_trees)

  line_reorderings = align_perms.readline().rstrip()
  line_sentence_trees = sentence_trees.readline().rstrip()

  if not line_reorderings: #EOF
    print "\nEnd of files reached"
    align_perms.close()
    sentence_trees.close()
    return [[],[]]

  else:
    reorderings_list = line_reorderings.split(" ")
    reorderings_list = [int(r) for r in reorderings_list]
    print reorderings_list
    print line_sentence_trees[1:-1]
    sentence_tree = Tree(line_sentence_trees[1:-1])
    return [reorderings_list, sentence_tree]


# given filename of the alignments and parses file, construct an ITG
# returns a list containing:
#   grammar rules dict: (A (B C)) --> frequency
#   lexical rules dict: (C dog) --> frequency
#   'inverted' lexical dict (keys are the lexical items): dog --> (B C D )
#   and, if the parameter 'probalistic' is set to true:
#     grammar rules left-hand-side dict: A --> frequency
#     lexical rules left-hand-side dict: C --> frequency
#   (otherwise these last 2 dicts are empty)
# 
# note: (temprarily) constructs a reorderings-permutations file from the alignments file,
# in current directory, so you need write permissions there
def fill_itg_from_files(fn_alignments, fn_sentence_trees, probalistic):  
  if not (os.path.isfile(fn_alignments) and os.path.isfile(fn_sentence_trees)):  
    print "%sfile %s and/or %s not found\n"% (output_symbol,fn_alignments, fn_sentence_trees)
    exit(1)
    
  itg_grammar_rules = defaultdict(int)
  itg_lexical_rules = defaultdict(int)
  itg_lexical_inv = defaultdict(list)

  itg_lhs_grammar_rules  = defaultdict(int)
  itg_lhs_lexical_rules  = defaultdict(int)

  fn_align_perms = filename_temp_reorderings
  reorder_command_output = subprocess.check_output("cat " + fn_alignments + " | sed \"s/[^ ]*-//g\" > " + fn_align_perms, shell=True)
  if not not reorder_command_output:
    raise Exception("alignments-file %s could not be converted to permutations-file %s" % fn_alignments, fn_align_perms)

  [align_perms, sentence_trees] = open_train_files(fn_align_perms, fn_sentence_trees)

  nr_lines = -1
  nr_processed = -1
  lines_read = ["init","init" ]
  while lines_read[0]:
    lines_read = [reorderings_list , sentence_tree] = load_next_line(align_perms, sentence_trees)
    if lines_read[0]:
      if nr_lines > 0 and nr_lines % 50 == 0:
        print "%s read line %d - %d\n" % (output_symbol, nr_lines-50, nr_lines)
        print "%s processed %d\n" % (output_symbol, nr_processed)
      nr_lines += 1
      if len(reorderings_list) > max_sentence_len:
        print "%s sentence was too long \n" % ""
      else:
        parse_chart = make_parse_chart(sentence_tree, reorderings_list)
        [grammar_rules, lexical_rules] = get_rules_from_parse_chart(parse_chart, sentence_tree)
        #  add rules
        for grammar_rule in grammar_rules:
          itg_grammar_rules[grammar_rule] += 1
          if probalistic:
            itg_lhs_grammar_rules[grammar_rule[0]] += 1

        for lexical_rule in lexical_rules:
          itg_lexical_rules[lexical_rule] += 1
          if not lexical_rule[0] in itg_lexical_inv[lexical_rule[1]]:
            itg_lexical_inv[lexical_rule[1]].append(lexical_rule[0])

          if probalistic:
            itg_lhs_lexical_rules[lexical_rule[0]] += 1
        nr_processed += 1
  
  
  try: # remove constructed align_perms file
    os.remove(filename_temp_reorderings)
  except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
  except:
    print "Unexpected error:", sys.exc_info()[0]
    raise

  return [itg_grammar_rules, itg_lexical_rules, itg_lexical_inv, itg_lhs_grammar_rules, itg_lhs_lexical_rules]


# convert ITG to Probalistic ITG
# this means in this case: frequencies become probabilities
# 'itg_lhs_rules' should contain the frequencies of all occurred left hand side's in 'itg_rules'
def convert_to_pitg(itg_rules, itg_lhs_rules):
  for key, value in itg_rules.iteritems():
    itg_rules[key] /= float(itg_lhs_rules[key[0]])


# write the ITG to a grammar and lexicon file in bitpar format
# see http://stp.lingfil.uu.se/~nivre/master/statmet4.html
def to_bitpar_files(grammar_rules, lexical_rules, lexical_inv):
  try:
    bitpar_grammar = open(filename_bitpar_grammar, "wb")
    bitpar_lexicon  = open(filename_bitpar_lexicon, "wb")

    bitpar_grammar.write("%s %s %s\n" % (bitpar_top_prob, bitpar_top, bitpar_top_child))
    for key, value in grammar_rules.iteritems():
      bitpar_grammar.write("%s %s %s %s\n" %( value , key[0] , key[1][0] , key[1][1]))

    for key, value in lexical_inv.iteritems():
      bitpar_lexicon.write("%s" % key)
      for lhs in value:
        count_or_prob = lexical_rules[(lhs, key)]
        bitpar_lexicon.write("\t%s\t%s" % ( lhs, count_or_prob))
      bitpar_lexicon.write("\n")

    bitpar_grammar.close()
    bitpar_lexicon.close()

  except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
  except:
    print "Unexpected error:", sys.exc_info()[0]
    raise

# construct a (p)itg
# write it to bitpar files
def construct_itg(probalistic=False) :

  [itg_grammar_rules, itg_lexical_rules, itg_lexical_inv, itg_lhs_grammar_rules, itg_lhs_lexical_rules] = \
                fill_itg_from_files(filename_alignments, filename_sentence_trees, probalistic)

  print itg_lexical_rules
  print itg_lhs_lexical_rules

  if probalistic:
    convert_to_pitg(itg_grammar_rules, itg_lhs_grammar_rules)
    convert_to_pitg(itg_lexical_rules, itg_lhs_lexical_rules)

  to_bitpar_files(itg_grammar_rules, itg_lexical_rules, itg_lexical_inv)

  

if __name__ == '__main__':  

  parser = argparse.ArgumentParser(description='Training System')
  parser.add_argument('-a','--alignments', help='Alignments file file name', required=True)
  parser.add_argument('-p','--parsetrees', help='Parse trees file filename',required=True)
  parser.add_argument('-d', '--debug',action='store_true', default=False,help='SHow debug info prints')
  parser.add_argument('-m', '--maxlen', action='store_const', help='Maximum length of sentences to be trained on',const=12)
  
  args = parser.parse_args()
  if not args.alignments or not args.parsetrees:
    print 'Error: please provide the filenames of the alignments and the (source) parse trees files'
  else:
    filename_alignments = args.alignments
    filename_sentence_trees = args.parsetrees
    construct_itg(probalistic=False)
