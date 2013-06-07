from nltk import Tree
import re
from nltk.tree import *
from nltk.draw import tree
from  nltk import treetransforms

inv_suffix = "<"


def get_reorderings_list(sentence_tree, rlist=[], index=0, pos=0):
  if rlist == []:
    rlist = [0]*len(sentence_tree.leaves())
  if not isinstance(sentence_tree, tree.Tree) or  len(sentence_tree) == 1: # leaf
    rlist[index] = pos
    return index

  else :

    head = sentence_tree.node

    left = sentence_tree[0]
    right = sentence_tree[1]
    addleft = 0
    addright = len(left)

    if (head[-len(inv_suffix):] == inv_suffix):
      temp = right
      right = left
      left = temp
      temp = addright
      addright = addleft
      addleft = temp

    #print "left:%s, right:%s" % (left, right)
    #print "rlist: %s" % rlist

    index = get_reorderings_list(left, rlist, index, pos+addleft)
    index = get_reorderings_list(right, rlist, index+1, pos+addright)

    return index



tree_string = Tree('(S (NP (N man)) (VP< (V bites) (NP (N dog))))')
#tree_string = Tree('(A (B< a b) (C c d))')
reordered = [0]*len(tree_string.leaves())
get_reorderings_list(tree_string,reordered)
print reordered
