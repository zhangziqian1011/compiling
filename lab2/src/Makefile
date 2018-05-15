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
OUTFILES=$(CMMFILES:.cmm=.out)

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
	rm -f $(OUTFILES)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
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
	../parser ../testcase/test11.cmm 1> ../testcase/test11.out
	../parser ../testcase/test12.cmm 1> ../testcase/test12.out
	../parser ../testcase/test13.cmm 1> ../testcase/test13.out
	../parser ../testcase/test14.cmm 1> ../testcase/test14.out
	../parser ../testcase/test15.cmm 1> ../testcase/test15.out
	../parser ../testcase/test16.cmm 1> ../testcase/test16.out
	../parser ../testcase/test17.cmm 1> ../testcase/test17.out
	../parser ../testcase/test18.cmm 1> ../testcase/test18.out
	../parser ../testcase/test19.cmm 1> ../testcase/test19.out
	../parser ../testcase/test20.cmm 1> ../testcase/test20.out
	../parser ../testcase/test21.cmm 1> ../testcase/test21.out
	../parser ../testcase/test22.cmm 1> ../testcase/test22.out
	../parser ../testcase/test23.cmm 1> ../testcase/test23.out
	../parser ../testcase/test24.cmm 1> ../testcase/test24.out

submit: clean
	zip -r ../../151220166_lab2.zip ../
	





