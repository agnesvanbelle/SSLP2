
# TYPE is either debug, prof, static or the empty string
TYPE = 

# Try different definitions of SGILIB if hash_map is not found
SGILIB=-DSGI__gnu_cxx
#SGILIB=-DSGIext
#SGILIB=

#AC = -DPRINT_ASSOC_SCORES

CXX = g++
WARNING = -Wall -Wcast-qual -Wconversion -Wno-deprecated

ifeq ($(TYPE),debug)
  DEBUG = -ggdb
  OPTIMIZATION = -O0
else
  DEBUG = -DNDEBUG
  ifeq ($(TYPE),prof)
    OPTIMIZATION = -O2
    PROFILE = -pg
  else
    OPTIMIZATION = -O3 -ffast-math
    ifeq ($(TYPE),static)
      STATIC = -static
    endif
  endif
endif

CFLAGS = $(DEBUG) $(PROFILE) $(OPTIMIZATION) $(WARNING) $(SGILIB) $(BITS) $(AC)
LDFLAGS = $(DEBUG) $(PROFILE) $(STATIC) $(LOPT)

CXXFLAGS = $(CFLAGS)
LXXFLAGS = $(LDFLAGS)

PROGRAM = bitpar


OBJ = Grammar.o apply.o po-apply.o bitop.o baseparser.o process.o vitparser.o \
	io.o Entry.o BaseLexicon.o Guesser.o LexSmoother.o SuffixLexicon.o \
	Lexicon.o estimate-freqs.o lexicalize.o parser.o traces.o quote.o \
	nbest.o

.PHONY: all clean archive realclean

all: $(PROGRAM)

bitpar: $(OBJ)

%: $(OBJ) %.o
	$(CXX) -o $@ $^ $(LDFLAGS)


clean:
	-rm -f *.o *~ Makefile.bak man1/*~ 2>&- > /dev/null

realclean: clean
	-rm -f $(PROGRAM) 2>&- > /dev/null

archive: 
	-gtar -zhcf BITPAR-`date '+%y%m%d'`.tar.gz *.[Chyl] Makefile man1/*

Makefile: *.C *.h
	-makedepend -Y -- $(CFLAGS) -- *.C 2>/dev/null 


# DO NOT DELETE

apply.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h SymbolTable.h
apply.o: prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h StringSet.h
apply.o: io.h Guesser.h WordClass.h SuffixLexicon.h
BaseLexicon.o: BaseLexicon.h SymbolTable.h sgi.h Entry.h StringSet.h io.h
baseparser.o: baseparser.h Grammar.h sgi.h bitop.h traces.h SymbolTable.h
baseparser.o: prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h
baseparser.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
bitop.o: bitop.h
bitpar.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
bitpar.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h
bitpar.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h vitparser.h
Entry.o: Entry.h SymbolTable.h sgi.h StringSet.h io.h
estimate-freqs.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
estimate-freqs.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h
estimate-freqs.o: Entry.h StringSet.h io.h Guesser.h WordClass.h
estimate-freqs.o: SuffixLexicon.h
Grammar.o: basic-functions.h Grammar.h sgi.h bitop.h traces.h SymbolTable.h
Grammar.o: prob.h
Guesser.o: Guesser.h WordClass.h io.h BaseLexicon.h SymbolTable.h sgi.h
Guesser.o: Entry.h StringSet.h SuffixLexicon.h
io.o: io.h
lexicalize.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
lexicalize.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h
lexicalize.o: Entry.h StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
Lexicon.o: Lexicon.h LexSmoother.h BaseLexicon.h SymbolTable.h sgi.h Entry.h
Lexicon.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
LexSmoother.o: LexSmoother.h BaseLexicon.h SymbolTable.h sgi.h Entry.h
LexSmoother.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
nbest.o: quote.h parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
nbest.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h
nbest.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
parser.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
parser.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h
parser.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h print.h
parser.o: quote.h print-YAP.h
po-apply.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
po-apply.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h
po-apply.o: Entry.h StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
process.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
process.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h
process.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
SuffixLexicon.o: SuffixLexicon.h SymbolTable.h sgi.h io.h Entry.h StringSet.h
traces.o: parser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
traces.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h Entry.h
traces.o: StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
vitparser.o: vitparser.h baseparser.h Grammar.h sgi.h bitop.h traces.h
vitparser.o: SymbolTable.h prob.h Lexicon.h LexSmoother.h BaseLexicon.h
vitparser.o: Entry.h StringSet.h io.h Guesser.h WordClass.h SuffixLexicon.h
vitparser.o: quote.h
