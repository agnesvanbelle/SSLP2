#get gold standard alignments from the alignments file
#input a-b ....
#output b gold standard reordering.
def get_gold_alignments(alignments_file, output_file):
	align_f = open(alignments_file, 'r')
	out_f = open(output_file, 'w')
	for line in align_f:
		newline=""
		for pair in line.split():
			pos = pair.index('-')
			newline+=str(pair[pos+1:] + ' ')
		newline = newline.strip()
		out_f.write('%s\n' % newline)
	align_f.close()
	out_f.close()
	

#input : the output from the moses lm query 
#output : a file containing the lm probability for each sentence
def get_lm_probs(lm_query_output, output_file):
	lm_f = open(lm_query_output, 'r')
	out_f = open(output_file, 'w')
	s = 'Total: '
	for line in lm_f:
		start_pos = line.index(s)
		end_pos = line.index('OOV:')
		newline = line[start_pos+len(s):end_pos-1]
		out_f.write('%s\n' % newline)
	
	lm_f.close()
	out_f.close()
	
#get_gold_alignments('europarl-v7.nl-en.test.align.en-en2', 'gold_align')
#get_lm_probs('lm_query.txt', 'lm_probs')

from nltk import Tree
import re
def tree_to_reordered(tree, inv_extension, index = 0):
    """Reorders a sentences according to its itg-tree
    
    Keyword arguments:
    tree -- nltk tree
    inv_extension -- extension denoting whether a node is inverted
    
    Returns reordered string, indexes and number of leaves"""
    pattern = '%s' % inv_extension # match if contains string
    if not isinstance(tree, Tree): # if terminal node
        return tree, [index], index+1
    elif len(tree)==1: # if unary rule
        return tree_to_reordered(tree[0], inv_extension, index)
    else: # if binary rule
        left_string, left_indexes, index = tree_to_reordered(tree[0],
            inv_extension, index)
        right_string, right_indexes, index = tree_to_reordered(tree[1],
            inv_extension, index)
        if re.search(pattern, tree.node): # if inverted rule
            reordered_string = '%s %s' % (right_string, left_string)
            right_indexes.extend(left_indexes)
            reordered_indexes = right_indexes
        else:
            reordered_string = '%s %s' % (left_string, right_string)
            left_indexes.extend(right_indexes)
            reordered_indexes = left_indexes

        return reordered_string, reordered_indexes, index

tree_string = Tree('(TOP (S (NP_inv (NNP Ms.) (NNP Haag) ) (VP (VBZ plays) (NP (NNP Elianti) ) ) (. .) ) ) ')
reordered_string, reordered, _ =  tree_to_reordered(tree_string, "inv")
print reordered_string
print reordered