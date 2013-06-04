import subprocess
from nltk import Tree
inv = '-inv'

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
get_pitpar_parses(k, binary_rules, unary_rules,test_sentences, parses_filename)
	
	f_out = open(output_filename, 'w')
	bitpar_command = 'bitpar -b '+k+' '+unary_rules+' '+binary_rules+' '+test_sentences
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
	f_parses = open(parses, 'r')
	f_sentences = open(sentences, 'w')
	f_indexes = open(indexes, 'w')
	for parse in parses:
		#empty line
		if parse == '':
			f_sentences.write('\n')
			f_indexes.write('\n')
		sentence, indexes = get_sentence_indexes(parse)
		f_sentences.write(sentence+'\n')
		f_indexes.write(indexes+'\n')
	
	f_parses.close()
	f_sentences.close()
	f_indexes.close()

#input:
	#binary_rules : grammar binary rules 
	#unary_rules : grammar unary rules
	#test_sentences: test sentences in 'normal format'
	#k : k best parses
#output:
	#files containing the k-best parses for each sentence (sentences, indexes, probabilities)
def testing(binary_rules, unary_rules,test_sentences, sentences, indexes, probabilities, k):
	sentences_reformed = 'bitpar_sentences.txt'
	reform_sentences(test_sentences, sentences_reformed)
	parses = k+'-best_parses'
	get_pitpar_parses(k, binary_rules, unary_rules,sentences_reformed, parses)
	
	#sentences: the file to which we output the k-best sentences
	#indexes : the file to which we output the k-best indexes
	#probabilites : the file to which we output the k-best probabilities
	get_sentence_indexes_from_parses(parses, sentences, indexes, probabilites, k)
	