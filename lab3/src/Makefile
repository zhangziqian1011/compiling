#TOOLS
CC=gcc
FLEX=flex
BISON=bison
CFLAGS=-std=c99

#FILES
CFILES=$(shell find ./ -maxdepth 1 -name "*.c")
OBJS=$(CFILES:.c=.o)
LFILE=$(shell find ./ -name "*.l")
YFILE=$(shell find ./ -name "*.y")
LFC=$(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC=$(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO=$(LFC:.c=.o)
YFO=$(YFC:.c=.o)

CMMFILES=$(shell find ../testcase -name "*.cmm")
OUTFILES=$(CMMFILES:.cmm=.ir)

#TARGETS
compiler: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o ../compiler $(filter-out $(LFO),$(OBJS)) -lfl -ly
parser: syntax $(filter-out $(LFO),$(OBJS))
		$(CC) -o ../parser $(filter-out $(LFO),$(OBJS)) -lfl -ly
syntax: lexical syntax-c
		$(CC) -c $(YFC) -o $(YFO)
lexical: $(LFILE)
		$(FLEX) -o $(LFC) $(LFILE)
syntax-c: $(YFILES)
		$(BISON) -o $(YFC) -d -v $(YFILE)
-include $(patsubst %.o, %.d, $(OBJS))

.PHONY: clean test

clean:
	rm -f ../compiler lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(OUTFILES)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~

test: compiler
	../compiler ../testcase/test1.cmm 1> ../testcase/test1.ir
	../compiler ../testcase/test2.cmm 1> ../testcase/test2.ir
	../compiler ../testcase/test3.cmm 1> ../testcase/test3.ir
	../compiler ../testcase/test4.cmm 1> ../testcase/test4.ir

submit: clean
	zip -r ../../151220166_lab2.zip ../
	





