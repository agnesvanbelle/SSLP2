/*******************************************************************/
/*      File: bitpar.C                                             */
/*    Author: Helmut Schmid                                        */
/*   Purpose:                                                      */
/*   Created: Tue Oct 29 10:59:29 2002                             */
/*  Modified: Thu Oct 25 16:39:16 2012 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#include <time.h>

#include <iostream>
using std::cerr;

#include "parser.h"
#include "vitparser.h"

static bool WithLemmas=false;
static bool PrintHeads=false;
static bool Viterbi=false;
static bool PrintOpt=false;
static bool PrintYAP=false;
static bool ViterbiProbs=false;
static bool InsideProbs=false;
static bool EstimatedFreqs=false;
static bool PrintTraces=false;
static bool Training=false;
static bool PrintRuleNumbers=false;
static bool PrintRuleProbs=false;
static bool PrintSuffixTrees=false;
static bool MaxFScore=false;
static bool MikeOption=false;
static double PruningThreshold=0.0;
static double SmoothingWeight=1.0;
static double MinLexProb=0.001;
static int  MaxSuffixLength=7;
static double GuesserGainThreshold=1.0;
static int  NBest=0;

static char *StartSymbol=NULL;
static char *Tfileprefix=NULL;
static char *OCfilename=NULL;
static char *OCfilename2=NULL;
static char *WCfilename=NULL;
static char TraceStartSymbol='*';
static char TraceEndSymbol='*';

extern FILE  *yyin;
extern int yyparse (void);

/*FA****************************************************************/
/*                                                                 */
/*  usage                                                          */
/*                                                                 */
/*FE****************************************************************/

void usage()

{
  cerr << "\nUsage:  bitpar grammar-file lexicon-file [infile [outfile]]\n";
  cerr << "OPTIONS\n";
  cerr << "-o   print parse forest\n";
  cerr << "-s s Use s as start symbol rather than the first symbol in the grammar file  \n";
  cerr << "-v   print Viterbi parse\n";
  cerr << "-u   file containing possible tags of unknown (lowercase) words\n";
  cerr << "-U   file containing possible tags of unknown capitalised words\n";
  cerr << "-w   file containing an automaton for word classification\n";
  cerr << "-S w set wordclass smoothing weight to w (default is 1)\n";
  cerr << "-tg  grammar contains trace symbols of the form *XY*\n";
  cerr << "-ts xy change the trace start symbol to x and the trace end symbol to y; traces have the form x...y\n";
  cerr << "-mlp t each tag in the lexicon must be more probable than t times the probability of the most likely tag\n";
  cerr << "-H   heads of grammar rules are marked with a preceding ^\n";
  cerr << "-lh  print lexical heads\n";
  cerr << "-l   lexicon entries with lemmata\n";
  cerr << "-b n print the n best parse trees\n";
  cerr << "-vp  print parse forests with Viterbi probabilities\n";
  cerr << "-ip  print parse forests with Inside probabilities\n";
  cerr << "-f   print parse forests with estimated frequencies\n";
  cerr << "-em f EM training. f ist the prefix of the files, where the output is stored.\n";
  cerr << "-prune t pruning of the parse forest with threshold t\n";
  cerr << "-t   print trace probabilities\n";
  cerr << "-rn  print parse forests with rule numbers\n";
  cerr << "-rp  print parse forests with rule probabilities (only works together with option -y)\n";
  cerr << "-mf  print the maximum estimated f-score parse\n";
  cerr << "-pst print the suffix trees of the unknown word guesser to a file named \"suffix-trees.txt\"\n";
  cerr << "-gg t Threshold for suffix tree pruning in the guesser is t (default 1)\n";
  cerr << "-y   print parse forest in YAP format\n";
  cerr << "-q   suppress status messages\n";
  cerr << "-i   verbose mode\n";
  cerr << "-h   this message\n";
  cerr << "Option -t implies option -tg.\n";
  exit(1);
}


/*FA****************************************************************/
/*                                                                 */
/*  get_flags                                                      */
/*                                                                 */
/*FE****************************************************************/

static void get_flags( int &argc, char **argv )

{
  for( int i=1; i<argc; i++ ) {
    if (strcmp(argv[i],"-q") == 0) {
      Quiet = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-vp") == 0) {
      ViterbiProbs = true;
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-ip") == 0) {
      InsideProbs = true;
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-f") == 0) {
      EstimatedFreqs = true;
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-tg") == 0) {
      WithTraces = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-rn") == 0) {
      PrintRuleNumbers = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-rp") == 0) {
      PrintRuleProbs = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-pst") == 0) {
      PrintSuffixTrees = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-mf") == 0) {
      MaxFScore = true;
      Viterbi = true;
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-t") == 0) {
      PrintTraces = true;
      WithTraces = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-v") == 0) {
      Viterbi = true;
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-o") == 0) {
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-y") == 0) {
      PrintYAP = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-l") == 0) {
      WithLemmas = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-H") == 0) {
      WithHeads = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-lh") == 0) {
      PrintHeads = true;
      WithHeads = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-h") == 0) {
      usage();
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-i") == 0) {
      Verbose = true;
      argv[i] = NULL;
    }
    else if (strcmp(argv[i],"-mike") == 0) {
      MikeOption = true;
      Viterbi = true;
      ViterbiProbs = true;
      PrintOpt = true;
      argv[i] = NULL;
    }
    else if (i < argc-1) {
      if (strcmp(argv[i],"-s") == 0) {
	StartSymbol = argv[i+1];
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-u") == 0) {
	OCfilename = argv[i+1];
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-U") == 0) {
	OCfilename2 = argv[i+1];
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-w") == 0) {
	WCfilename = argv[i+1];
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-S") == 0) {
	SmoothingWeight = atof(argv[i+1]);
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-mlp") == 0) {
	MinLexProb = atof(argv[i+1]);
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-gg") == 0) {
	GuesserGainThreshold = atof(argv[i+1]);
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-ts") == 0) {
	if (strlen(argv[i+1]) != 2) {
	  fprintf(stderr,"Error: option -ts requires a 2-letter argument!\n");
	  exit(1);
	}
	TraceStartSymbol = argv[i+1][0];
	TraceEndSymbol = argv[i+1][1];
	WithTraces = true;
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-b") == 0) {
	NBest = atoi(argv[i+1]);
	if (NBest < 1) {
	  fprintf(stderr,"Error: argument of option -b is out of range!\n");
	  exit(1);
	}
	PrintOpt = true;
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-em") == 0) {
	Training = true;
	Tfileprefix = argv[i+1];
	argv[i] = NULL;
	argv[++i] = NULL;
      }
      else if (strcmp(argv[i],"-prune") == 0) {
	PruningThreshold = atof(argv[i+1]);
	if (PruningThreshold <= 0.0 || PruningThreshold > 1) {
	  fprintf(stderr, "Error: invalid pruning threshold \"%s\"!\n", 
		  argv[i+1]);
	  exit(1);
	}
	argv[i] = NULL;
	argv[++i] = NULL;
      }
    }
  }
  // remove flags from the argument list
  int k;
  for( int i=k=1; i<argc; i++)
    if (argv[i] != NULL)
      argv[k++] = argv[i];
  argc = k;
}



/*******************************************************************/
/*                                                                 */
/*  main                                                           */
/*                                                                 */
/*******************************************************************/

int main( int argc, char *argv[] )

{
  get_flags( argc, argv );

  if (argc < 3)
    usage();
  
  FILE *gfile;
  if ((gfile = fopen(argv[1], "rt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", argv[1]);
    exit(1);
  }

  FILE *lfile;
  if ((lfile = fopen(argv[2], "rt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", argv[2]);
    exit(1);
  }
    
  FILE *infile = stdin;
  if (argc >= 4 && (infile = fopen(argv[3], "rt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", argv[3]);
    exit(1);
  } 
  
  FILE *outfile = stdout;
  if (argc >= 5 && (outfile = fopen(argv[4], "wt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", argv[4]);
    exit(1);
  }
  
  FILE *OCfile = NULL;
  if (OCfilename != NULL && (OCfile = fopen(OCfilename, "rt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", OCfilename);
    exit(1);
  }
  
  FILE *OCfile2 = NULL;
  if (OCfilename2 != NULL && (OCfile2 = fopen(OCfilename2, "rt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", OCfilename2);
    exit(1);
  }
  
  FILE *WCfile = NULL;
  if (WCfilename != NULL && (WCfile = fopen(WCfilename, "rt")) == NULL) {
    fprintf(stderr, "\nError: unable to open file \"%s\"\n\n", WCfilename);
    exit(1);
  }
    
  try {
    clock_t start;
    if (Viterbi && !NBest) {
      VitParser parser( gfile, lfile, StartSymbol, OCfile, OCfile2, WCfile, 
			TraceStartSymbol, TraceEndSymbol, SmoothingWeight,
			MinLexProb, GuesserGainThreshold, MaxSuffixLength, 
			WithLemmas, PrintSuffixTrees );

      parser.PrintProbs = ViterbiProbs;
      parser.MaxFScore = MaxFScore;
      fclose(gfile);
      fclose(lfile);
      if (MikeOption) {
	fputs("Startup finished. Ready for processing.\n", outfile);
	fflush(outfile);
      }
      parser.verbose = Verbose;
      start = clock();
      for(;;) {
	if (MikeOption)
	  start = clock();
	parser.next_parse(infile);
	if (parser.finished)
	  break;
	parser.print_parse(outfile);
	if (MikeOption) {
	  fprintf( outfile, "cpu-time=%.3fs\n", 
		   (double) (clock() - start) / CLOCKS_PER_SEC);
	  fflush(outfile);
	}
      }
    }
    else {
      Parser parser( gfile, lfile, StartSymbol, OCfile, OCfile2, WCfile, 
		     TraceStartSymbol, TraceEndSymbol, SmoothingWeight,
		     MinLexProb, GuesserGainThreshold, MaxSuffixLength, 
		     WithLemmas, PrintSuffixTrees );
      parser.verbose = Verbose;
      parser.Lexicalized = PrintHeads;
      parser.Viterbi = Viterbi;
      parser.ViterbiProbs = ViterbiProbs;
      parser.PruningThreshold = (Prob)PruningThreshold;
      parser.InsideProbs = InsideProbs;
      parser.NBest = NBest;
      parser.EstimatedFreqs = EstimatedFreqs;
      parser.Training = Training;
      parser.PrintRuleNumbers = PrintRuleNumbers;
      parser.PrintRuleProbs = PrintRuleProbs;
      fclose(gfile);
      fclose(lfile);
    
      start = clock();
      for(;;) {
	parser.next_parse(infile);

	if (parser.finished)
	  break;
	if (PrintTraces)
	  parser.print_trace_probs(outfile);
	if (PrintYAP)
	  parser.print_YAP_parse(outfile);
	else if (PrintOpt) {
	  if (NBest)
	    parser.print_nbest_parses( outfile );
	  else if (Viterbi)
	    parser.print_best_parse(outfile);
	  else
	    parser.print_parse(outfile);
	}
      }

      if (Training) {
	FILE *file;
	char buffer[1000];
	sprintf(buffer, "%s.gram", Tfileprefix);
	if ((file = fopen(buffer,"wt")) == NULL)
	  fprintf(stderr, "Error: unable to open file \"%s\"!\n", buffer);
	parser.grammar.store(file);
	fclose(file);
    
	sprintf(buffer, "%s.lex", Tfileprefix);
	if ((file = fopen(buffer,"wt")) == NULL)
	  fprintf(stderr, "Error: unable to open file \"%s\"!\n", buffer);
	parser.lexicon.write(file);
	fclose(file);
      }
    }
	
    if (!Quiet)
      fprintf( stderr, "\nraw cpu time %.3f\n", 
	       (double) (clock() - start) / CLOCKS_PER_SEC);
  }
  catch(const char* p) {
    cerr << "\nError: " << p << "\n\n";
  }
  
  if (!Quiet)
    cerr << "finished\n";
}
