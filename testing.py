import subprocess
from nltk import Tree
import os
import sys
import argparse

bitpar_top = 'TOP'
inv_suffix = '<'

#input: test sentences
#output: test sentences in bitpar format (one token per line, new line after each sentence)
def reform_sentences_bitpar(sentences, bitpar_sentences):
	f_in = open(sentences,'r')
	f_out = open(bitpar_sentences, 'w')
	for sentence in f_in:
		for word in sentence.split():
			if word == '(' or word == ')':	
				continue
			f_out.write(word + '\n')
		f_out.write('\n')
	f_in.close()
	f_out.close()
	
#reform_sentences_bitpar('europarl-v7.nl-en.test.en', 'bitpar_sentences')

#bitpar has to be installed in the machine
#TODO: provide instructions in the README
def get_bitpar_parses(k, binary_rules, unary_rules,test_sentences, parses):
	f_out = open(parses, 'w')
	bitpar_command = './bitpar -b '+str(k)+' -s S '+binary_rules+' '+unary_rules+' '+test_sentences+' '+parses + ' -vp'
	p = subprocess.Popen(bitpar_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	retval = p.wait()
	f_out.close()


#get the reorderings recursively for an ITG tree
#see get_sentence_and_indexes
#
#parameters rlist and slist should be initialized to the proper length of the sentence!
#e.g. rlist = [0]*len(sentence_tree.leaves())
def get_sentence_and_indexes_rec_helper(sentence_tree, rlist, slist, index=0, pos=0):
  
  if not isinstance(sentence_tree, Tree) or  len(sentence_tree) == 1: # leaf or unary rule
    rlist[index] = pos   
    slist[index] = sentence_tree.leaves()[0] if isinstance(sentence_tree, Tree) else sentence_tree
    return index

  else :
    left = sentence_tree[0]
    right = sentence_tree[1]
    addleft = 0
    addright = 1
    if isinstance(right, Tree):
      addright = len(left.leaves())   

    if (sentence_tree.node[-len(inv_suffix):] == inv_suffix): #if inversed rule
      right, left = left, right
      addright, addleft = addleft, addright    

    index = get_sentence_and_indexes_rec_helper(left, rlist, slist, index, pos+addleft)
    index = get_sentence_and_indexes_rec_helper(right, rlist, slist, index+1, pos+addright)

    return index


#given the binary parse tree (from our reordering ITG) in form of a string,
#returns the reordered sentence and the reorderings_list (a list of indices)
def get_sentence_and_indexes(parsed_sentence):

  sentence_tree = Tree(parsed_sentence)
  if sentence_tree.node == bitpar_top: #remove designated TOP-symbol    
    sentence_tree = sentence_tree[0]
    
  rlist = [0]*len(sentence_tree.leaves())
  slist = [""]*len(sentence_tree.leaves())
  get_sentence_and_indexes_rec_helper(sentence_tree, rlist, slist)
  reordered_sentence = " ".join(slist)
  
  return reordered_sentence, rlist


#input is the 'dirty' bitpar parse
def get_bitpar_probabilities(bitpar_parses, bitpar_probs):
	f_in = open(bitpar_parses, 'r')
	f_out = open(bitpar_probs, 'w')
	for line in f_in:
		if 'logvitprob' in line:
			pos = line.index('=')
			prob = line[pos+1:]
			f_out.write(prob) 
		elif line=='\n': #if we have two empty lines it means that we dont have the parse
			f_out.write('\n')
		elif 'No parse' in line:
			f_out.write('nil\n')	
	f_in.close()
	f_out.close()
	
#post-process.pl has to be installed in the machine
#TODO: provide instructions in the README
def get_cleaned_bitpar_parses(parses, cleaned_parses):
	temp_parses = 'temp_parses'
	command = 'perl post-process.pl ' + parses + ' ' + temp_parses 
	p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT) 
	retval = p.wait()
	
	f_in = open(temp_parses, 'r')
	f_out = open(cleaned_parses, 'w')
	previous_empty = False
	for line in f_in:
		#reached next set of parses
		if line == '\n':
			if previous_empty:		
				previous_empty = False
				f_out.write('\n')
			else:
				previous_empty = True
		#not parsed line
		elif line == '(' + bitpar_top + ' nil) \n':
			f_out.write('\nnil\n')
		else:
			f_out.write(line)
			previous_empty = False
		
	f_out.close()
	f_in.close()
	os.remove(temp_parses)
	
#get_cleaned_bitpar_parses('bitpar_output', 'clean')
#
#creates the reordered sentences and indexes files
#parses is the cleaned version from bitpar
def get_sentence_indexes_from_parses(parses, sentences, indexes,k):

	f_parses = open(parses, 'r')
	f_sentences = open(sentences, 'w')
	f_indexes = open(indexes, 'w')
	#previous_empty = False
	for parse in f_parses:
		#empty line
		if parse == '\n':
			f_sentences.write('\n')
			f_indexes.write('\n')
			#previous_empty = True
		elif 'nil' in parse:
			#print 'nil'
			#break
			f_sentences.write('nil\n')
			f_indexes.write('nil\n')
		else:
			#"print parse"
			sentence, indexes = get_sentence_and_indexes(parse)
			f_sentences.write(str(sentence)+'\n')
			indexes_string =''
			prefix =''
			for i in indexes:
				indexes_string += prefix+str(i)
				prefix=' '
			f_indexes.write(indexes_string+'\n')
	
	f_parses.close()
	f_sentences.close()
	f_indexes.close()


#TODO: provide instructions in the README
#runs moses script for getting lm probabilities and outputs them to lm_probs
def get_moses_lm_probs(sentences, lm_probs, lm):
	temp  = 'temp'
	bitpar_command = '/home/sslp13/moses/bin/query '+ lm+' < ' + sentences + ' >' + temp
	
	p = subprocess.Popen(bitpar_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	retval = p.wait()
	f_in = open(temp, 'r')
	f_out = open(lm_probs, 'w')
	s = 'Total:'
	for line in f_in:
		start_pos = line.index(s)
		end_pos = line.index('OOV:')
		newline = line[start_pos+1+len(s):end_pos-1]
		f_out.write(newline+'\n')
	f_out.close()
	os.remove(temp)
	f_in.close()
	

def get_best_lm(list):
	sorted_list = sorted(list, key=lambda x:-x[2])
	return sorted_list[0][0]

def get_best_combined(list):
	sorted_list = sorted(list, key=lambda x:-(x[1]+x[2]))
	return sorted_list[0][0]

#return one file with the best parses according to LM and 
#one file with the best parses according to the trained model + LM
def get_most_probable_alignments(indexes, model_probs, lm_probs, out_lm, out_model_and_lm):
	f_indexes = open(indexes,'r')
	f_m_probs = open(model_probs, 'r')
	f_lm_probs = open(lm_probs, 'r')

	#read the arrays
	sentences=[]
	list=[]
	for indexes in f_indexes:
		try:
			m_prob = f_m_probs.readline()
		except Exception:
			print 'error'
		if indexes != '\n':
			lm_prob = f_lm_probs.readline()
			#ignore it if nill
			if indexes != 'nil\n':
				list.append((indexes, float(m_prob), float(lm_prob)))
		else:
			sentences.append(list)
			list = []
	f_indexes.close()
	f_m_probs.close()
	f_lm_probs.close()	

	f_out_lm = open(out_lm, 'w')
	f_out_model_lm = open(out_model_and_lm, 'w')
	#for each sentence		
	for list in sentences:
		if not list:
			f_out_lm.write('nil\n')	
			f_out_model_lm.write('nil\n')
		else:
			f_out_lm.write(get_best_lm(list))	
			f_out_model_lm.write(get_best_combined(list))
	f_out_lm.close()
	f_out_model_lm.close()

def check_file(filename):
	try:
	   with open(filename): pass
	except IOError:
		print 'Error: filename ' +filename+' not in current directory'
		sys.exit()
    
    
#input:
	#binary_rules : grammar binary rules 
	#unary_rules : grammar unary rules
	#test_sentences: test sentences in 'normal format'
	#k : k best parses
	#lm : the path to the trained language model 
#output:
	#return one file with reorderings, one for each method (LM, LM+model) 
def testing(binary_rules, unary_rules,test_sentences, k, lm, out_lm, out_combined):
	check_file('bitpar')
	check_file('post-process.pl')
	sentences_reformed = 'bitpar_sentences.txt'
	parses = str(k)+'-best_parses'
	#sentences: the file to which we output the k-best sentences
	#indexes : the file to which we output the k-best indexes
	#probabilites : the file to which we output the k-best probabilities
	clean_parses = parses+'_clean'
	model_probs = 'bitpar_probabilities'
	lm_probs = 'lm_probabilities'
	out_lm = str(k) + '_'+out_lm
	out_combined  = str(k)+'_'+out_combined
	indexes='indexes'
	sentences = 'output_sentences'
	
	reform_sentences_bitpar(test_sentences, sentences_reformed)
	print 'Parsing with bitpar... '+str(k)+'-best parses'
	get_bitpar_parses(k, binary_rules, unary_rules,sentences_reformed, parses)
	os.remove(sentences_reformed)
	print 'Done.'

	print 'Preprocessing...'
	get_bitpar_probabilities(parses, model_probs)
	get_cleaned_bitpar_parses(parses, clean_parses)
	os.remove(parses)
	print 'Done.'
	
	print 'Creating sentences and indexes files..'
	get_sentence_indexes_from_parses(clean_parses, sentences, indexes, k)
	print 'Done.'
	print 'Getting LM probabilities with Moses...'
	get_moses_lm_probs(sentences, lm_probs, lm)	
	print 'Done.'
	print 'Getting most probable alignments with the two methods'
	get_most_probable_alignments(indexes, model_probs, lm_probs, out_lm, out_combined)
	print 'Done.'
	
	os.remove(clean_parses)
	os.remove(lm_probs)
	os.remove(sentences)
	os.remove(indexes)
	
if __name__ == '__main__':
  #tree_string = '(TOP (A (B< (G (L l) (F (K< (X x) (Y y)) (I (J j)) ) ) (H h)) (C< c d)))'
  #s, r = get_sentence_and_indexes(tree_string)
  #print s
  #print r  
  
	parser = argparse.ArgumentParser(description='Testing phase')
	parser.add_argument('-b', '--grammar_binary',help='Extracted grammar binary rules',required=True)
	parser.add_argument('-u', '--grammar_unary',help='Extracted grammar unary rules',required=True)
	parser.add_argument('-k', help='k-best parses',required=True)
	parser.add_argument('-t', '--test_sentences',help='Test sentences',required=True)
	parser.add_argument('-lm', '--language_model',help='Path to the language model',required=True)
	parser.add_argument('--lm_only',help='Output to best parse only according to the language model',required=True)
	parser.add_argument('--combined',help='Output to best parse combining the language model with the trained model',required=True)
	args = parser.parse_args() 
	testing(args.grammar_binary, args.grammar_unary, args.test_sentences, args.k, args.language_model, args.lm_only, args.combined)

#testing('../new.dat','../bitpar_lexicon_12.dat', '../heldout/europarl-v7.nl-en.test.en', 20, '/home/sslp13/data/sourceReordering/lm/*.blm', 'best_lm', 'best_combined')
