import sys
from nltk.tree import *
from nltk.draw import tree
from  nltk import treetransforms

inv_extension = "-Inv"

def make_syntax_chart(sentence_tree,  sc, col=0, row=None) :
  

  if row == None :
    row = len(sentence_tree.leaves())
  

      
  if isinstance(sentence_tree, tree.Tree):
    #print "%s %d %d" % (sentence_tree.node, col, row-1)
    
    if (col, row-1) in sc :
      sc[(col, row-1)] = sentence_tree.node + ":" + sc[(col, row-1)]
    else:
      sc[(col, row-1)] =  sentence_tree.node        
   
    
    lengthkids = 0
    for i in range (0, len(sentence_tree)):
      
      lengthkidsprev = lengthkids
      
      if isinstance(sentence_tree[i], tree.Tree):
        lengthkids += len(sentence_tree[i].leaves())
        length = len(sentence_tree[i].leaves())
      else:
        lengthkids += 1
        length = 1
        
      make_syntax_chart(sentence_tree[i], sc, col+lengthkidsprev, length + col+lengthkidsprev)
      #print "%s  %s  %d" % (sentence_tree[i], sentence_tree[i].node ,len(sentence_tree[i]))
    
  else :
    #print sentence_tree
    return
  
def printdict(d) :
  for key, value in d.iteritems():    
    print "%s --> %s" % (key, value)

# span should be a tuple where span[1] >= span[0]
def valid_phrase(span, reorderings_list):
  len_phrase = max(reorderings_list[span[0]:span[1]+1]) - min(reorderings_list[span[0]:span[1]+1])  
  return len_phrase == span[1]-span[0]
  
def make_parse_chart(sentence_tree, reorderings_list) :
  
  sc = {}
  make_syntax_chart(sentence_tree, sc)  
  
  printdict(sc)
  
  len_sentence = len(sentence_tree.leaves())
  
  for span_size in range(0, len_sentence) :
    for span_startpos in range (0, len_sentence-span_size):
      span_endpos = span_startpos + span_size
      span = (span_startpos, span_endpos)
      if valid_phrase(span, reorderings_list):
        print "span_size: %d \n span: %s " % (span_size, span)
        
        for split_startpos in range (0, span_size):
          split_left = (span_startpos , span_startpos+ split_startpos)
          split_right = (span_startpos+split_startpos+1 , span_endpos)
          print "l: %s , r: %s " % (split_left, split_right)
          
          
          # get leftkids
          # get rightkids, from parse chart
          
          # for all combi's of them
            # get parent in syntax chart (create on if it isn't there)
            # check for inversion of left and right kid acc. to reorderings
                
      else :
        print "span %s is not a valid phrase " % (span,)
  
def test() :
  sentence_tree = Tree('(S (NP (N man)) (VP (V bites) (NP (N dog))))')
  #sentence_tree = Tree('(S (ADVP (RB next)) (, ,) (NP (PRP we)) (VP (VBP have) (NP (NP (NNS flags)) (PP (IN of) (NP (NN convenience))))) (. .)) ')
  #sentence_tree = Tree(" (S (NP (DT this)) (VP (VBZ is) (ADVP (RB no) (RB longer)) (ADJP (JJ tolerable))) (. .)) ")
  treetransforms.chomsky_normal_form(sentence_tree, factor='right', horzMarkov=None, vertMarkov=0, childChar='|', parentChar='^')
  
  reorderings_list = [2, 1, 0]
  sentence_list = sentence_tree.leaves()
  
  #print list(enumerate(sentence_tree))
  
  syntax_chart = {}
  
  #print sentence_tree[0]
  
 
   
  make_parse_chart(sentence_tree, reorderings_list)  
  
  sentence_tree.draw()
  
  
if __name__ == '__main__':   
  test()
