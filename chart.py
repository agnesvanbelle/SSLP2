import sys
from nltk.tree import *
from nltk.draw import tree
from  nltk import treetransforms

inv_extension = "-Inv"

def make_syntax_chart(sentence_tree,  sc, nr=0, length=None) :
  

  if length == None :
    length = len(sentence_tree.leaves())
  

      
  if isinstance(sentence_tree, tree.Tree):
    #print "%s %d %d" % (sentence_tree.node, nr, length-1)
    
    if (nr, length-1) in sc :
      sc[(nr, length-1)] = sentence_tree.node + ":" + sc[(nr, length-1)]
    else:
      sc[(nr, length-1)] =  sentence_tree.node
        
   
    
    lengthkids = 0
    for i in range (0, len(sentence_tree)):
      
      lengthkidsprev = lengthkids
      
      if isinstance(sentence_tree[i], tree.Tree):
        lengthkids += len(sentence_tree[i].leaves())
        l = len(sentence_tree[i].leaves())
      else:
        lengthkids += 1
        l = 1
        
      make_syntax_chart(sentence_tree[i], sc, nr+lengthkidsprev, l + nr+lengthkidsprev)
      #print "%s  %s  %d" % (sentence_tree[i], sentence_tree[i].node ,len(sentence_tree[i]))
    
  else :
    #print sentence_tree
    return
  
def printdict(d) :
  for key, value in d.iteritems():    
    print "%s --> %s" % (key, value)
  
  
def make_parse_chart(sentence_tree, reorderings_list) :
  
  sc = {}
  make_syntax_chart(sentence_tree, sc)
  
  printdict(sc)
  
 
  
  
def test() :
  #sentence_tree = Tree('(S (NP (N man)) (VP (V bites) (NP (N dog))))')
  #sentence_tree = Tree('(S (ADVP (RB next)) (, ,) (NP (PRP we)) (VP (VBP have) (NP (NP (NNS flags)) (PP (IN of) (NP (NN convenience))))) (. .)) ')
  sentence_tree = Tree(" (S (NP (DT this)) (VP (VBZ is) (ADVP (RB no) (RB longer)) (ADJP (JJ tolerable))) (. .)) ")
  treetransforms.chomsky_normal_form(sentence_tree, factor='right', horzMarkov=None, vertMarkov=0, childChar='|', parentChar='^')
  
  reorderings_list = [2, 0, 1]
  sentence_list = sentence_tree.leaves()
  
  #print list(enumerate(sentence_tree))
  
  syntax_chart = {}
  
  #print sentence_tree[0]
  
 
   
  make_parse_chart(sentence_tree, reorderings_list)  
  
  sentence_tree.draw()
  
  
if __name__ == '__main__':   
  test()
