import os
import subprocess
import sys
import argparse
#get gold standard alignments from the alignments file
#input a-b ....
#output b gold standard reordering.
def get_gold_single_alignments(alignments_file, output_file):
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

#script must be in the same folder
def run_kendall_script(f_gold_new, f_test_new):
	command='./kendall_tau.pl ' + f_gold_new  + ' ' + f_test_new 
	p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
 	for line in	p.stdout.readlines():
		print line
	retval = p.wait()

def eval_kendall_tau(gold_alignments, test_alignments):
	temp_alignments  ='temp_alignments'
	get_gold_single_alignments(gold_alignments, temp_alignments)
	
	f_gold = open(temp_alignments, 'r')
	f_test = open(test_alignments, 'r')
	f_gold_new = open('gold_new', 'w')
	f_test_new = open('test_new', 'w')
	average = 0
	total=0
	for gold in f_gold:
		test = f_test.readline()
		if test == 'nil\n':
			continue
		#dist = distance(gold, test)
		if len(gold.split()) != len(test.split()):
			continue
		f_gold_new.write(gold)
		f_test_new.write(test)
	
	os.remove(temp_alignments)
	f_gold.close()
	f_test.close()
	f_test_new.close()
	f_gold_new.close()
	run_kendall_script('gold_new', 'test_new')
	os.remove('gold_new')
	os.remove('test_new')

def hamming_distance(gold, test):
	diffs = 0
	for ch1, ch2 in zip(gold, test):
			if ch1 == ch2:
					diffs += 1
	return float(diffs)/len(gold)

def check_file(filename):
	try:
	   with open(filename): pass
	except IOError:
		print 'Error: filename ' +filename+' not in current directory'
		sys.exit()

def eval_hamming(gold_alignments, test_alignments):
	check_file('kendall_tau.pl')
	temp_alignments  ='temp_alignments'
	get_gold_single_alignments(gold_alignments, temp_alignments)
	f_gold = open(temp_alignments, 'r')
	f_test = open(test_alignments, 'r')	
	total = 0
	num=0
	for gold in f_gold:
		test = f_test.readline()
		if test == 'nil\n':
			continue
		if len(gold.split()) != len(test.split()):
			continue
		total+=hamming_distance(gold, test)
		num+=1
	
	print 'Average (1-)Hamming distance is : ' + str(total/num)
	
	f_gold.close()
	f_test.close()
	os.remove(temp_alignments)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Evaluate System')
	parser.add_argument('--kendall',action='store_true', default=False,help='Calculate Kendall tau distance')
	parser.add_argument('--hamming',action='store_true', default=False, help='Calculate Hamming distance')
	parser.add_argument('-g', '--gold_standard',help='gold_standard',required=True)
	parser.add_argument('-b', '--best_parse',help='best_parse',required=True)
	args = parser.parse_args()
	if not args.kendall and not args.hamming:
		print 'Error: please provide the distance metric you want to be calculated'
	if args.kendall:
		eval_kendall_tau(args.gold_standard,args.best_parse)
	if args.hamming:
		eval_hamming(args.gold_standard,args.best_parse)