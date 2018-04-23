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

CMM=$(shell find ../testcase -name "*.cmm")
OUTS=$(CMM:.cmm=.out)

#TARGETS
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
	rm -f ../parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f $(OUTS)
	rm -f *~

test: parser
	../parser ../testcase/test1.cmm 1> ../testcase/test1.out
	../parser ../testcase/test2.cmm 1> ../testcase/test2.out
	../parser ../testcase/test3.cmm 1> ../testcase/test3.out
	../parser ../testcase/test4.cmm 1> ../testcase/test4.out
	../parser ../testcase/test5.cmm 1> ../testcase/test5.out
	../parser ../testcase/test6.cmm 1> ../testcase/test6.out
	../parser ../testcase/test7.cmm 1> ../testcase/test7.out
	../parser ../testcase/test8.cmm 1> ../testcase/test8.out
	../parser ../testcase/test9.cmm 1> ../testcase/test9.out
	../parser ../testcase/test10.cmm 1> ../testcase/test10.out

submit: clean
	zip -r ../../151220166_lab1.zip ../
	





