import subprocess
from nltk import Tree
suffix = '<'

#input: test sentences
#output: test sentences in bitpar format (one token per line, new line after each sentence)
def reform_sentences_bitpar(sentences, bitpar_sentences):
	f_in = open(sentences,'r')
	f_out = open(bitpar_sentences, 'w')
	for sentence in f_in:
		for word in sentence.split():
			f_out.write('%s\n' % word)
		f_out.write('\n')
	f_in.close()
	f_out.close()
	
#reform_sentences_bitpar('europarl-v7.nl-en.test.en', 'bitpar_sentences')

#bitpar has to be installed in the machine
#TODO: provide instructions in the README
def get_bitpar_parses(k, binary_rules, unary_rules,test_sentences, parses):
	f_out = open(output_filename, 'w')
	bitpar_command = 'bitpar -b '+k+' '+unary_rules+' '+binary_rules+' '+test_sentences+' '+parses + ' -vp'
	p = subprocess.Popen(bitpar_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	for line in iter(p.stdout.readline, ''):
		f_out.write(line)
	retval = p.wait()
	f_out.close()

#recursively print the sentences and the indexes from the parse tree
def get_sentence_indexes_rec(parse_tree, x):
	#leaf node
	#x is the current 
	if not isinstance(parse_tree, Tree):
		return parse_tree, [x], x+1
	#unary 
	elif len(parse_tree)==1:
		return get_sentence_indexes_rec(parse_tree[0], x)
	#binary
	else:
		sentence_LHS, indexes_LHS, x = get_sentence_indexes_rec(parse_tree[0], x)
		#TODO: check why it is 'x' again
		sentence_RHS, indexes_RHS, x = get_sentence_indexes_rec(parse_tree[1], x)
		#the rule is inverted
		if inv in parse_tree.node:
			sentence = sentence_RHS + " " + sentence_LHS
			#get the new indexes by concatenating the LHS to the LHS
			indexes_RHS.extend(indexes_LHS)
			indexes = indexes_RHS
		else:
			sentence = sentence_LHS + " " + sentence_RHS
			#get the new indexes by concatenating the RHS to the LHS
			indexes_LHS.extend(indexes_RHS)
			indexes = indexes_LHS
		#print sentence
		return sentence, indexes, x

def get_sentence_indexes(parsed_sentence):
	return get_sentence_indexes_rec(Tree(parsed_sentence), 0)
	
#tree_string = '(TOP (S (NP (NNP Ms.) (NNP Haag) ) (VP (VBZ plays) (NP (NNP Elianti) ) ) (. .) ) ) '
#sentence, indexes, x = get_sentence_indexes(tree_string)


#creates the reordered sentences and indexes files
def get_sentence_indexes_from_parses(parses, sentences, indexes, probabilites,k):
	
	#TODO: write to dictionaries instead of files???
	#OR use the files and delete them afterwards
	f_parses = open(parses, 'r')
	f_sentences = open(sentences, 'w')
	f_probs = open(probabilities, 'w')
	f_indexes = open(indexes, 'w')
	for parse in parses:
		#empty line
		if parse == '':
			f_sentences.write('\n')
			f_indexes.write('\n')
			f_probs.write('\n')
		if 'loginv' in parse:
			pos = parse.index('logvit')
			prob = parse[pos+1]
			f_probs.write(prob)
		sentence, indexes = get_sentence_indexes(parse)
		f_sentences.write(sentence+'\n')
		f_indexes.write(indexes+'\n')
	
	f_parses.close()
	f_sentences.close()
	f_indexes.close()
	f_probs.close()


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

get_lm_probs('../lm_query.txt', 'lm_probs')


#TODO: provide instructions in the README
#TODO: should the sentences file have empty lines for denoting next set of parses?
#runs moses script for getting lm probabilities and outputs them to lm_probs
def get_moses_lm_probs(sentences, lm_probs, lm):
	f_out = open(output_filename, 'w')
	bitpar_command = 'query' #TODO add the correct command
	p = subprocess.Popen(bitpar_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	for line in iter(p.stdout.readline, ''):
		start_pos = line.index(s)
		end_pos = line.index('OOV:')
		newline = line[start_pos+len(s):end_pos-1]
		f_out.write(newline+'\n')
	retval = p.wait()
	f_out.close()
	
from collections import Counter
def create_unknown_tags(bitpar_lexicon, unknown):
	f_in = open(bitpar_lexicon, 'r')
	f_out = open(unknown, 'w')
	cnt = Counter()
	for line in f_in:
		splitted = line.split()
		if len(splitted) == 3 and splitted[2] == '1':
			cnt[splitted[1]] += 1
	
	for tag in cnt:
		f_out.write(tag +'\n')
		#f_out.write(tag + '\t'+ str(cnt[tag]) +'\n')	
	f_in.close()
	f_out.close()

#create_unknown_tags('../bitpar_lexicon.dat', '../unknown.dat')


#return a file with the best parses according to LM and 
#a file with the best parses according to the trained model + LM
#def get_most_probable_alignments(indexes, model_probs, lm_probs, out_lm, out_model_and_lm)

	#for each 'block'
		#get the best lm and output the indexes to out_lm
		#add the log prob of lm + model to get the best parse and output the indexes to out_model_and_lm

#input:
	#binary_rules : grammar binary rules 
	#unary_rules : grammar unary rules
	#test_sentences: test sentences in 'normal format'
	#k : k best parses
	#lm : the path to the trained language model 
#output:
	#files containing the k-best parses for each sentence (sentences, indexes, probabilities)
def testing(binary_rules, unary_rules,test_sentences, k, lm):
	sentences_reformed = 'bitpar_sentences.txt'
	reform_sentences(test_sentences, sentences_reformed)
	parses = k+'-best_parses'
	get_bitpar_parses(k, binary_rules, unary_rules,sentences_reformed, parses)

	#sentences: the file to which we output the k-best sentences
	sentences='sentences'
	#indexes : the file to which we output the k-best indexes
	indexes='indexes'
	#probabilites : the file to which we output the k-best probabilities
	model_probs = 'probabilities'
	get_sentence_indexes_from_parses(parses, sentences, indexes, model_probs, k)
	lm_probs = 'lm_probabilities'
	get_sentence_lm_probs(sentences, lm_probs, lm)	
	out_lm = 'best_lm'
	out_model_and_lm  = 'best_model_and_lm'
	get_most_probable_alignments(indexes, model_probs, lm_probs, out_lm, out_model_and_lm)